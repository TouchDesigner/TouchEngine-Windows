/* Shared Use License: This file is owned by Derivative Inc. (Derivative)
* and can only be used, and/or modified for use, in conjunction with
* Derivative's TouchDesigner software, and only if you are a licensee who has
* accepted Derivative's TouchDesigner license or assignment agreement
* (which also govern the use of this file). You may share or redistribute
* a modified version of this file provided the following conditions are met:
*
* 1. The shared file or redistribution must retain the information set out
* above and this list of conditions.
* 2. Derivative's name (Derivative Inc.) or its trademarks may not be used
* to endorse or promote products derived from this file without specific
* prior written permission from Derivative.
*/

#include "stdafx.h"
#include "OpenGLRenderer.h"
#include "Strings.h"
#include <TouchEngine/TouchEngine.h>
#include <TouchEngine/TEOpenGL.h>

static void ThrowIfFalse(BOOL result)
{
	if (!result)
	{
		throw std::runtime_error("OpenGL Renderer error");
	}
}

static void ThrowIfNull(void* result)
{
	if (!result)
	{
		throw std::runtime_error("OpenGL Renderer error");
	}
}

const char *OpenGLRenderer::VertexShader = "#version 330\n\
in vec2 vertCoord; \
in vec2 texCoord; \
out vec2 fragTexCoord; \
void main() { \
	fragTexCoord = texCoord; \
	gl_Position = vec4(vertCoord, 1.0, 1.0); \
}";


const char *OpenGLRenderer::FragmentShader = "#version 330\n\
uniform sampler2D tex; \
in vec2 fragTexCoord; \
out vec4 color; \
void main() { \
	color = texture(tex, fragTexCoord); \
}";

static void GLAPIENTRY
MessageCallback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
		(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
		type, severity, message);
}

OpenGLRenderer::OpenGLRenderer()
{
}


OpenGLRenderer::~OpenGLRenderer()
{
}

void
OpenGLRenderer::setup(HWND window)
{
	Renderer::setup(window);
	myDC = GetDC(window);
	PIXELFORMATDESCRIPTOR format{ 0 };
	format.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	format.nVersion = 1;
	format.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
	format.iPixelType = PFD_TYPE_RGBA;
	format.cColorBits = 32;
	format.cDepthBits = 0;
	format.iLayerType = PFD_MAIN_PLANE;
	int selected = ChoosePixelFormat(myDC, &format);

	ThrowIfFalse(SetPixelFormat(myDC, selected, &format));
	myRenderingContext = wglCreateContext(myDC);
	ThrowIfNull(myRenderingContext);
	ThrowIfFalse(wglMakeCurrent(myDC, myRenderingContext));

	if (glewInit() != GLEW_OK)
	{
		throw std::runtime_error("glewInit failed");
	}

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, nullptr);
	
	{
		const GLubyte* render = glGetString(GL_RENDERER);
		myDeviceName = reinterpret_cast<const char *>(render);
	}
	
	{
		RECT client;
		GetClientRect(window, &client);
		glViewport(0, 0, client.right, client.bottom);

		ThrowIfFalse(myProgram.build(VertexShader, FragmentShader));
	}
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	
	glUseProgram(myProgram.getName());
	GLint tex = glGetUniformLocation(myProgram.getName(), "tex");
	glUniform1i(tex, 0);

	myVAIndex = glGetAttribLocation(myProgram.getName(), "vertCoord");
	myTAIndex = glGetAttribLocation(myProgram.getName(), "texCoord");

	glUseProgram(0);
	
	ThrowIfFalse(myOutputImage.setup(myVAIndex, myTAIndex));
	
	if (TEOpenGLContextCreate(myDC, myRenderingContext, myContext.take()) != TEResultSuccess)
	{
		throw std::runtime_error("Couldn't create TEOpenGLContext");
	}
}

bool
OpenGLRenderer::configure(TEInstance* instance, std::string& error)
{
	if (TEOpenGLContextSupportsTexturesForInstance(myContext, instance))
	{
		return Renderer::configure(instance, error);
	}
	error = "OpenGL is not supported. The selected GPU does not have needed features.";
	error += "\nThe selected GPU is: ";
	error += myDeviceName;
	return false;
}

void
OpenGLRenderer::resize(int width, int height)
{
	Renderer::resize(width, height);

	wglMakeCurrent(myDC, myRenderingContext);

	glViewport(0, 0, width, height);

	wglMakeCurrent(nullptr, nullptr);
}

void
OpenGLRenderer::stop()
{
	wglMakeCurrent(myDC, myRenderingContext);

	Renderer::stop();
	
	myProgram.destroy();

	if (myRenderingContext)
	{
		wglMakeCurrent(nullptr, nullptr);
		wglDeleteContext(myRenderingContext);
	}
}

bool
OpenGLRenderer::render()
{
	wglMakeCurrent(myDC, myRenderingContext);
	
	glClearColor(myBackgroundColor.red, myBackgroundColor.green, myBackgroundColor.blue, myBackgroundColor.alpha);
	
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(myProgram.getName());

	if (myOutputImage.isValid())
	{
		myOutputImage.fit(static_cast<float>(myWidth), static_cast<float>(myHeight));
		myOutputImage.position(0.0f, 0.0f);
		myOutputImage.draw();
	}

	glUseProgram(0);
	
	glFlush();

	SwapBuffers(myDC);

	wglMakeCurrent(nullptr, nullptr);
	return true;
}

TouchObject<TETexture> OpenGLRenderer::getTexture(const unsigned char* rgba, size_t bytesPerRow, int width, int height)
{
	wglMakeCurrent(myDC, myRenderingContext);
	
	myInputTexture = OpenGLTexture(rgba, bytesPerRow, width, height);
	
	wglMakeCurrent(nullptr, nullptr);

	// Create a reference-counted reference to the same texture
	OpenGLTexture* copied = new OpenGLTexture(myInputTexture);

	TouchObject<TEOpenGLTexture> texture;
	texture.take(TEOpenGLTextureCreate(copied->getName(),
		GL_TEXTURE_2D,
		GL_RGBA8,
		copied->getWidth(),
		copied->getHeight(),
		TETextureOriginBottomLeft,
		kTETextureComponentMapIdentity,
		textureReleaseCallback,
		copied));
	return texture;
}

const std::string& OpenGLRenderer::getDeviceName() const
{
	return myDeviceName;
}

void
OpenGLRenderer::clearInputs()
{
	myInputTexture = OpenGLTexture();
	Renderer::clearInputs();
}

bool OpenGLRenderer::setOutputImage(const TouchObject<TETexture>& texture, const TouchObject<TESemaphore>&, uint64_t)
{
	setOutputImage(texture);
	return true;
}

void OpenGLRenderer::setOutputImage(const TouchObject<TETexture>& texture)
{
	const auto& source = myOutputImage.getTexture().getSource();
	if (source)
	{
		TEOpenGLTextureUnlock(source);
	}
	
	Renderer::setOutputImage(texture);

	myOutputImage.update(OpenGLTexture());

	if (texture && TETextureGetType(texture) == TETextureTypeD3DShared)
	{
		TouchObject<TEOpenGLTexture> created;
		if (TEOpenGLContextGetTexture(myContext, static_cast<TED3DSharedTexture*>(texture.get()), created.take()) == TEResultSuccess)
		{
			if (TEOpenGLTextureLock(created) == TEResultSuccess)
			{
				myOutputImage.update(OpenGLTexture(created));
			}
		}
	}
}

void
OpenGLRenderer::clearOutputs()
{
	myOutputImage = OpenGLImage();
	Renderer::clearOutputs();
}

void
OpenGLRenderer::textureReleaseCallback(GLuint texture, TEObjectEvent event, void *info)
{
	// Delete our reference to the texture (and the texture itself if we are the last reference)
	if (event == TEObjectEventRelease)
	{
		delete reinterpret_cast<OpenGLTexture*>(info);
	}
}

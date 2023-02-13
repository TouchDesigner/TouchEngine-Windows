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

bool
OpenGLRenderer::setup(HWND window)
{
	bool success = Renderer::setup(window);
	if (success)
	{
		myDC = GetDC(window);
		PIXELFORMATDESCRIPTOR format{ 0 };
		format.nSize = sizeof(PIXELFORMATDESCRIPTOR);
		format.nVersion = 1;
		format.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
		format.iPixelType = PFD_TYPE_RGBA;
		format.cColorBits = 32;
		format.cDepthBits = 32; // TODO: or not
		format.iLayerType = PFD_MAIN_PLANE;
		int selected = ChoosePixelFormat(myDC, &format);

		success = SetPixelFormat(myDC, selected, &format) ? true : false;
		if (success)
		{
			myRenderingContext = wglCreateContext(myDC);
		}
		if (!myRenderingContext)
		{
			success = false;
		}
		if (success)
		{
			success = wglMakeCurrent(myDC, myRenderingContext) ? true : false;
		}
	}
	if (success)
	{
		if (glewInit() != GLEW_OK)
		{
			success = FALSE;
		}
	}
	if (success)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(MessageCallback, nullptr);
	}
	if (success)
	{
		const GLubyte* render = glGetString(GL_RENDERER);
		myDeviceName = ConvertToWide(reinterpret_cast<const char *>(render));
	}
	if (success)
	{
		RECT client;
		GetClientRect(window, &client);
		glViewport(0, 0, client.right, client.bottom);

		success = myProgram.build(VertexShader, FragmentShader);
	}
	if (success)
	{
		glUseProgram(myProgram.getName());
		GLint tex = glGetUniformLocation(myProgram.getName(), "tex");
		glUniform1i(tex, 0);

		myVAIndex = glGetAttribLocation(myProgram.getName(), "vertCoord");
		myTAIndex = glGetAttribLocation(myProgram.getName(), "texCoord");

		glUseProgram(0);
	}
	if (success)
	{
		if (TEOpenGLContextCreate(myDC, myRenderingContext, myContext.take()) != TEResultSuccess)
		{
			success = false;
		}
	}
	return success;
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
	
	glClearColor(myBackgroundColor[0], myBackgroundColor[1], myBackgroundColor[2], 1.0);
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glUseProgram(myProgram.getName());

	float scale = 1.0f / (max(myInputImages.size(), myOutputImages.size()) + 1.0f);

	drawImages(myInputImages, scale, -0.5f);
	drawImages(myOutputImages, scale, 0.5f);

	glUseProgram(0);
	
	glFlush();

	SwapBuffers(myDC);

	wglMakeCurrent(nullptr, nullptr);
	return true;
}

size_t
OpenGLRenderer::getInputImageCount() const
{
	return myInputImages.size();
}

void
OpenGLRenderer::addInputImage(const unsigned char * rgba, size_t bytesPerRow, int width, int height)
{
	wglMakeCurrent(myDC, myRenderingContext);
	
	myInputImages.emplace_back();
	myInputImages.back().setup(myVAIndex, myTAIndex);
	myInputImages.back().update(OpenGLTexture(rgba, bytesPerRow, width, height));
	
	wglMakeCurrent(nullptr, nullptr);

	Renderer::addInputImage(rgba, bytesPerRow, width, height);
}

bool
OpenGLRenderer::getInputImage(size_t index, TouchObject<TETexture> & texture, TouchObject<TESemaphore> & semaphore, uint64_t & waitValue)
{
	if (inputDidChange(index))
	{
		// Create a reference-counted reference to the same texture
		OpenGLTexture* copied = new OpenGLTexture(myInputImages[index].getTexture());

		TEOpenGLTexture* out = TEOpenGLTextureCreate(copied->getName(),
			GL_TEXTURE_2D,
			GL_RGBA8,
			copied->getWidth(),
			copied->getHeight(),
			TETextureOriginBottomLeft,
			kTETextureComponentMapIdentity,
			textureReleaseCallback,
			copied);
		texture.take(out);
		// The TEOpenGLContext handles sync for us, so we needn't set semaphore or waitValue
		markInputUnchanged(index);
		return true;
	}
	return false;
}

bool OpenGLRenderer::releaseOutputImage(size_t index, TouchObject<TETexture>& texture, TouchObject<TESemaphore>& semaphore, uint64_t& waitValue)
{
	if (index < myOutputImages.size())
	{
		const auto& source = myOutputImages.at(index).getTexture().getSource();
		if (source)
		{
			TEOpenGLTextureUnlock(source);
		}
	}
	return false;
}

void OpenGLRenderer::acquireOutputImage(size_t index, TouchObject<TESemaphore>& semaphore, uint64_t& waitValue)
{
	if (index < myOutputImages.size())
	{
		const auto& source = myOutputImages.at(index).getTexture().getSource();
		if (source)
		{
			TEOpenGLTextureLock(source);
		}
	}
}

const std::wstring& OpenGLRenderer::getDeviceName() const
{
	return myDeviceName;
}

void
OpenGLRenderer::clearInputImages()
{
	myInputImages.clear();
}

void
OpenGLRenderer::addOutputImage()
{
	wglMakeCurrent(myDC, myRenderingContext);

	myOutputImages.emplace_back();
	myOutputImages.back().setup(myVAIndex, myTAIndex);

	wglMakeCurrent(nullptr, nullptr);

	Renderer::addOutputImage();
}

void
OpenGLRenderer::setOutputImage(size_t index, const TouchObject<TETexture>& texture)
{
	bool success = false;
	if (TETextureGetType(texture) == TETextureTypeD3DShared)
	{
		TouchObject<TEOpenGLTexture> created;
		if (TEOpenGLContextGetTexture(myContext, static_cast<TED3DSharedTexture *>(texture.get()), created.take()) == TEResultSuccess)
		{
			myOutputImages.at(index).update(OpenGLTexture(created));
			
			// Retains it for us
			Renderer::setOutputImage(index, texture);
			success = true;
		}
	}

	if (!success)
	{
		myOutputImages.at(index).update(OpenGLTexture());
		Renderer::setOutputImage(index, nullptr);
	}
}

void
OpenGLRenderer::clearOutputImages()
{

	Renderer::clearOutputImages();
}

void
OpenGLRenderer::textureReleaseCallback(GLuint texture, TEObjectEvent event, void *info)
{
	// TODO: might come from another thread
	// Delete our reference to the texture (and the texture itself if we are the last reference)
	if (event == TEObjectEventRelease)
	{
		delete reinterpret_cast<OpenGLTexture*>(info);
	}
}

void
OpenGLRenderer::drawImages(std::vector<OpenGLImage>& images, float scale, float xOffset)
{
	float numImages = (1.0f / scale) - 1.0f;
	float spacing = 1.0f / numImages;
	float yOffset = 1.0f - spacing;
	float ratio = static_cast<float>(myHeight) / myWidth;
	for (auto &image : images)
	{
		image.scale(scale * ratio, scale);
		image.position(xOffset, yOffset);
		image.draw();
		yOffset -= spacing * 2;
	}
}

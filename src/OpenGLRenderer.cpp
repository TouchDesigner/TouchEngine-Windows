#include "stdafx.h"
#include "OpenGLRenderer.h"
#include <TouchEngine/TouchEngine.h>

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

bool OpenGLRenderer::setup(HWND window)
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
		if (TEOpenGLContextCreate(myDC, myRenderingContext, &myContext) != TEResultSuccess)
		{
			success = false;
		}
	}
	return success;
}

void OpenGLRenderer::resize(int width, int height)
{
	Renderer::resize(width, height);

    wglMakeCurrent(myDC, myRenderingContext);

    glViewport(0, 0, width, height);

    wglMakeCurrent(nullptr, nullptr);
}

void OpenGLRenderer::stop()
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

bool OpenGLRenderer::render()
{
	wglMakeCurrent(myDC, myRenderingContext);
    
	glClearColor(myBackgroundColor[0], myBackgroundColor[1], myBackgroundColor[2], 1.0);
    
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glUseProgram(myProgram.getName());

    float scale = 1.0f / (max(myLeftSideImages.size(), myRightSideImages.size()) + 1.0f);

    drawImages(myLeftSideImages, scale, -0.5f);
    drawImages(myRightSideImages, scale, 0.5f);

    glUseProgram(0);
    
	glFlush();

	SwapBuffers(myDC);

	wglMakeCurrent(nullptr, nullptr);
	return true;
}

size_t OpenGLRenderer::getLeftSideImageCount() const
{
	return myLeftSideImages.size();
}

void OpenGLRenderer::addLeftSideImage(const unsigned char * rgba, size_t bytesPerRow, int width, int height)
{
    wglMakeCurrent(myDC, myRenderingContext);
	
    myLeftSideImages.emplace_back();
    myLeftSideImages.back().setup(myVAIndex, myTAIndex);
    myLeftSideImages.back().update(OpenGLTexture(rgba, bytesPerRow, width, height));
    
    wglMakeCurrent(nullptr, nullptr);
}

TETexture * OpenGLRenderer::createLeftSideImage(size_t index)
{
    // Create a reference-counted reference to the same texture
	OpenGLTexture *copied = new OpenGLTexture(myLeftSideImages[index].getTexture());

    TEOpenGLTexture *out = TEOpenGLTextureCreate(copied->getName(), GL_TEXTURE_2D, GL_RGBA8, copied->getWidth(), copied->getHeight(), false, textureReleaseCallback, copied);
	return out;
}

void OpenGLRenderer::clearLeftSideImages()
{
    myLeftSideImages.clear();
}

void OpenGLRenderer::addRightSideImage()
{
    wglMakeCurrent(myDC, myRenderingContext);

    myRightSideImages.emplace_back();
    myRightSideImages.back().setup(myVAIndex, myTAIndex);

    wglMakeCurrent(nullptr, nullptr);

	Renderer::addRightSideImage();
}

void OpenGLRenderer::setRightSideImage(size_t index, const TouchObject<TETexture>& texture)
{
	bool success = false;
	if (TETextureGetType(texture) == TETextureTypeDXGI)
	{
		TouchObject<TEOpenGLTexture> created;
		if (TEOpenGLContextCreateTexture(myContext, static_cast<TEDXGITexture *>(texture.get()), created.take()) == TEResultSuccess)
		{
			myRightSideImages.at(index).update(OpenGLTexture(TEOpenGLTextureGetName(created),
				TEOpenGLTextureGetWidth(created),
				TEOpenGLTextureGetHeight(created),
				TETextureIsVerticallyFlipped(created),
				[created]() mutable {
					created.reset(); // release the TETexture when we are done with it
					// in fact reset() needn't be called, we just want created to last
				}));
			
			// Retains it for us
			Renderer::setRightSideImage(index, created);
			success = true;
		}
	}

	if (!success)
	{
		myRightSideImages.at(index).update(OpenGLTexture());
		Renderer::setRightSideImage(index, nullptr);
	}
}

void OpenGLRenderer::clearRightSideImages()
{

	Renderer::clearRightSideImages();
}

void OpenGLRenderer::textureReleaseCallback(GLuint texture, void *info)
{
    // TODO: might come from another thread
    // Delete our reference to the texture (and the texture itself if we are the last reference)
    delete reinterpret_cast<OpenGLTexture *>(info);
}

void OpenGLRenderer::drawImages(std::vector<OpenGLImage>& images, float scale, float xOffset)
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

#include "stdafx.h"
#include "OpenGLRenderer.h"
#include <TouchEngine.h>

OpenGLRenderer::OpenGLRenderer()
{
}


OpenGLRenderer::~OpenGLRenderer()
{
}

bool OpenGLRenderer::setup(HWND window)
{
	std::lock_guard<std::mutex> guard(myMutex);

	bool result = Renderer::setup(window);
	if (result)
	{
		HDC dc = GetDC(window);
		PIXELFORMATDESCRIPTOR format{ 0 };
		format.nSize = sizeof(PIXELFORMATDESCRIPTOR);
		format.nVersion = 1;
		format.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
		format.iPixelType = PFD_TYPE_RGBA;
		format.cColorBits = 32;
		format.cDepthBits = 32; // TODO: or not
		format.iLayerType = PFD_MAIN_PLANE;
		int selected = ChoosePixelFormat(dc, &format);

		result = SetPixelFormat(dc, selected, &format) ? true : false;
		if (result)
		{
			myRenderingContext = wglCreateContext(dc);
		}
		if (!myRenderingContext)
		{
			result = false;
		}
		if (result)
		{
			result = wglMakeCurrent(dc, myRenderingContext) ? true : false;
		}
	}
	if (result)
	{
		if (glewInit() != GLEW_OK)
		{
			result = FALSE;
		}
	}
	if (result)
	{
		RECT client;
		GetClientRect(window, &client);
		glViewport(0, 0, client.right, client.bottom);
	}
	return result;
}

void OpenGLRenderer::resize(int width, int height)
{
	// TODO: 
}

void OpenGLRenderer::stop()
{
	Renderer::stop();

	if (myRenderingContext)
	{
		wglMakeCurrent(nullptr, nullptr);
		wglDeleteContext(myRenderingContext);
	}
}

bool OpenGLRenderer::render()
{
	std::lock_guard<std::mutex> guard(myMutex);

	wglMakeCurrent(GetDC(getWindow()), myRenderingContext);

	glClearColor(myBackgroundColor[0], myBackgroundColor[1], myBackgroundColor[2], 1.0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glFlush();

	SwapBuffers(GetDC(getWindow()));

	wglMakeCurrent(nullptr, nullptr);
	return true;
}

size_t OpenGLRenderer::getLeftSideImageCount() const
{
	return myLeftSideImages.size();
}

void OpenGLRenderer::addLeftSideImage(const unsigned char * rgba, size_t bytesPerRow, int width, int height)
{
	myLeftSideImages.emplace_back(rgba, bytesPerRow, width, height);
}

TETexture * OpenGLRenderer::createLeftSideImage(size_t index)
{
	auto &texture = myLeftSideImages[index];
	//TEOpenGLTexture *texture = TEOpenGLTextureCreate(texture.getName(), texture.getWidth(), texture.getHeight(), GAH HERE YES)
	return nullptr;
}

void OpenGLRenderer::clearLeftSideImages()
{
}

void OpenGLRenderer::addRightSideImage()
{

	Renderer::addRightSideImage();
}

void OpenGLRenderer::setRightSideImage(size_t index, TETexture * texture)
{

	Renderer::setRightSideImage(index, texture);
}

void OpenGLRenderer::clearRightSideImages()
{

	Renderer::clearRightSideImages();
}

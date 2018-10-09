#pragma once
#include "Renderer.h"
#include <mutex>
#include <vector>
#include "OpenGLTexture.h"
#include "GL/glew.h"

class OpenGLRenderer :
	public Renderer
{
public:
	OpenGLRenderer();
	virtual ~OpenGLRenderer();
	virtual DWORD getWindowStyleFlags() const { return WS_CLIPCHILDREN | WS_CLIPSIBLINGS; }
	virtual bool setup(HWND window);
	virtual void resize(int width, int height) override;
	virtual void stop();
	virtual bool render();
	virtual size_t getLeftSideImageCount() const;
	virtual void addLeftSideImage(const unsigned char *rgba, size_t bytesPerRow, int width, int height);
	virtual TPTexture *createLeftSideImage(size_t index);
	virtual void clearLeftSideImages();
	virtual void addRightSideImage();
	virtual void setRightSideImage(size_t index, TPTexture *texture);
	virtual void clearRightSideImages();
private:
	std::mutex myMutex;
	HGLRC myRenderingContext = nullptr;
	std::vector<OpenGLTexture> myLeftSideImages;
};


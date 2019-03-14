#pragma once
#include "Renderer.h"
#include <mutex>
#include <vector>
#include "OpenGLImage.h"
#include "OpenGLProgram.h"
#include "GL/glew.h"

class OpenGLRenderer :
	public Renderer
{
public:
	OpenGLRenderer();
	virtual ~OpenGLRenderer();
	virtual DWORD getWindowStyleFlags() const { return WS_CLIPCHILDREN | WS_CLIPSIBLINGS; }
	virtual bool setup(HWND window);
    HGLRC getRC() const { return myRenderingContext; };
	virtual void resize(int width, int height) override;
	virtual void stop();
	virtual bool render();
	virtual size_t getLeftSideImageCount() const;
	virtual void addLeftSideImage(const unsigned char *rgba, size_t bytesPerRow, int width, int height);
	virtual TETexture *createLeftSideImage(size_t index);
	virtual void clearLeftSideImages();
	virtual void addRightSideImage();
	virtual void setRightSideImage(size_t index, TETexture *texture);
	virtual void clearRightSideImages();
private:
    static const char *VertexShader;
    static const char *FragmentShader;
    static void textureReleaseCallback(GLuint texture, void *info);
    void drawImages(std::vector<OpenGLImage>& images, float scale, float xOffset);
    OpenGLProgram myProgram;
    GLuint myVAO = 0;
    GLuint myVBO = 0;
    GLint myVAIndex = -1;
    GLint myTAIndex = -1;
	HGLRC myRenderingContext = nullptr;
	std::vector<OpenGLImage> myLeftSideImages;
    std::vector<OpenGLImage> myRightSideImages;
};


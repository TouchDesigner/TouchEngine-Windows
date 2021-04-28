#pragma once

#include <TouchEngine/TEGraphicsContext.h>
#include <TouchEngine/TETexture.h>
#include <vector>
#include <memory>

class Renderer
{
public:
    Renderer();
    virtual ~Renderer();
	HWND getWindow() const { return myWindow; }
	virtual DWORD getWindowStyleFlags() const { return 0; }
    virtual bool setup(HWND window);
	virtual void resize(int width, int height);
	virtual void stop();
    virtual bool render() = 0;
    void setBackgroundColor(float r, float g, float b);
	virtual size_t getLeftSideImageCount() const = 0;
	virtual void addLeftSideImage(const unsigned char *rgba, size_t bytesPerRow, int width, int height) = 0;
	virtual TETexture *createLeftSideImage(size_t index) = 0;
	virtual void clearLeftSideImages() = 0;
	size_t getRightSideImageCount();
	virtual void addRightSideImage();
	virtual void setRightSideImage(size_t index, TETexture *texture);
	virtual void clearRightSideImages();
	virtual TEGraphicsContext* getTEContext() const = 0;
protected:
    float myBackgroundColor[3];
	int myWidth{ 0 };
	int myHeight{ 0 };
private:
	HWND myWindow;
	std::vector<std::shared_ptr<TETexture *>> myRightSideImages;
};


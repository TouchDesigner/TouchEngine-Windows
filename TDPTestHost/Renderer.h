#pragma once

#include <TDP/TPTexture.h>
#include <vector>
#include <memory>

class Renderer
{
public:
    Renderer();
    virtual ~Renderer();
	HWND getWindow() const { return myWindow; }
    virtual bool setup(HWND window);
	virtual void stop();
    virtual bool render() = 0;
    void setBackgroundColor(float r, float g, float b);
	virtual size_t getLeftSideImageCount() const = 0;
	virtual void addLeftSideImage(const unsigned char *rgba, size_t bytesPerRow, int width, int height) = 0;
	virtual TPTexture *createLeftSideImage(size_t index) = 0;
	virtual void clearLeftSideImages() = 0;
	size_t getRightSideImageCount();
	virtual void addRightSideImage();
	virtual void setRightSideImage(size_t index, TPTexture *texture);
	virtual void clearRightSideImages();
protected:
    float myBackgroundColor[3];
private:
	HWND myWindow;
	std::vector<std::shared_ptr<TPTexture *>> myRightSideImages;
};


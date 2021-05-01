#include "stdafx.h"
#include "Renderer.h"


Renderer::Renderer()
    : myBackgroundColor{0.0f, 0.0f, 0.0f}
{
}


Renderer::~Renderer()
{
}

bool Renderer::setup(HWND window)
{
	myWindow = window;
	return true;
}

void Renderer::resize(int width, int height)
{
	myWidth = width;
	myHeight = height;
}

void Renderer::stop()
{
	myRightSideImages.clear();
}

void Renderer::setBackgroundColor(float r, float g, float b)
{
    myBackgroundColor[0] = r;
    myBackgroundColor[1] = g;
    myBackgroundColor[2] = b;
}

size_t Renderer::getRightSideImageCount()
{
	return myRightSideImages.size();
}

void Renderer::addRightSideImage()
{
	myRightSideImages.emplace_back();
}

void Renderer::setRightSideImage(size_t index, const TouchObject<TETexture> &texture)
{
	myRightSideImages[index] = texture;
}

void Renderer::clearRightSideImages()
{
	myRightSideImages.clear();
}

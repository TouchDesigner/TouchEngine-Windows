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

void Renderer::setRightSideImage(size_t index, TPTexture * texture)
{
	if (texture)
	{
		TPRetain(texture);

		myRightSideImages[index] = std::shared_ptr<TPTexture *>(new TPTexture *(texture), [](TPTexture **t) {
			TPRelease(*t);
			delete t;
		});
	}
	else
	{
		myRightSideImages[index] = std::shared_ptr<TPTexture *>();
	}
}

void Renderer::clearRightSideImages()
{
	myRightSideImages.clear();
}

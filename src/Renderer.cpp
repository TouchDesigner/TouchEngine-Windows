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
#include "Renderer.h"


Renderer::Renderer()
	: myBackgroundColor{0.0f, 0.0f, 0.0f}
{
}


Renderer::~Renderer()
{
}

bool
Renderer::setup(HWND window)
{
	myWindow = window;
	return true;
}

void
Renderer::resize(int width, int height)
{
	myWidth = width;
	myHeight = height;
}

void
Renderer::stop()
{
	myRightSideImages.clear();
}

void
Renderer::setBackgroundColor(float r, float g, float b)
{
	myBackgroundColor[0] = r;
	myBackgroundColor[1] = g;
	myBackgroundColor[2] = b;
}

size_t
Renderer::getRightSideImageCount()
{
	return myRightSideImages.size();
}

void
Renderer::addRightSideImage()
{
	myRightSideImages.emplace_back();
}

void
Renderer::setRightSideImage(size_t index, const TouchObject<TETexture> &texture)
{
	myRightSideImages[index] = texture;
}

void
Renderer::clearRightSideImages()
{
	myRightSideImages.clear();
}

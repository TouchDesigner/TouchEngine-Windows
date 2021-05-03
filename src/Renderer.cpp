/* Shared Use License: This file is owned by Derivative Inc. (Derivative) and
 * can only be used, and/or modified for use, in conjunction with
 * Derivative's TouchDesigner software, and only if you are a licensee who has
 * accepted Derivative's TouchDesigner license or assignment agreement (which
 * also govern the use of this file).  You may share a modified version of this
 * file with another authorized licensee of Derivative's TouchDesigner software.
 * Otherwise, no redistribution or sharing of this file, with or without
 * modification, is permitted.
 *
 * TouchEngine
 *
 * Copyright © 2021 Derivative. All rights reserved.
 *
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

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


Renderer::~Renderer() noexcept(false)
{
}

bool
Renderer::setup(HWND window)
{
	myWindow = window;
	return true;
}

bool Renderer::configure(TEInstance* instance, std::wstring &error)
{
	return true;
}

bool Renderer::doesInputTextureTransfer() const
{
	return false;
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
	myOutputImages.clear();
}

void
Renderer::setBackgroundColor(float r, float g, float b)
{
	myBackgroundColor[0] = r;
	myBackgroundColor[1] = g;
	myBackgroundColor[2] = b;
}

void Renderer::beginImageLayout()
{
}

void Renderer::addInputImage(const unsigned char* rgba, size_t bytesPerRow, int width, int height)
{
	myInputImageUpdates.push_back(true);
}

size_t
Renderer::getRightSideImageCount()
{
	return myOutputImages.size();
}

void
Renderer::addOutputImage()
{
	myOutputImages.emplace_back();
}

void Renderer::endImageLayout()
{
}

void
Renderer::setOutputImage(size_t index, const TouchObject<TETexture> &texture)
{
	myOutputImages[index] = texture;
}

const TouchObject<TETexture>& Renderer::getOutputImage(size_t index) const
{
	if (index < myOutputImages.size())
		return myOutputImages.at(index);
	static const TouchObject<TETexture> empty;
	return empty;
}

void
Renderer::clearOutputImages()
{
	myOutputImages.clear();
}

bool Renderer::inputDidChange(size_t index) const
{
	return myInputImageUpdates[index];
}

void Renderer::markInputChange(size_t index)
{
	myInputImageUpdates[index] = true;
}

void Renderer::markInputUnchanged(size_t index)
{
	myInputImageUpdates[index] = false;
}

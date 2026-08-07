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
#include <cmath>
#include "Renderer.h"

Renderer::Renderer()
{
}


Renderer::~Renderer() noexcept(false)
{
}

void
Renderer::setup(HWND window)
{
	myWindow = window;
}

bool Renderer::configure(TEInstance* instance, std::string &error)
{
	myMinBufferAlignment = TEInstanceGetMinimumBufferAlignment(instance);
	return true;
}

bool Renderer::doesInputResourceTransfer() const
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
	clearInputs();
	clearOutputs();
	myPendingTransfers.clear();
}

void
Renderer::setBackgroundColor(const Color &color)
{
	myBackgroundColor = color;
}

void Renderer::clearInputs()
{

}

void
Renderer::setOutputImage(const TouchObject<TETexture> &texture)
{
	myOutputImage = texture;
}

void Renderer::addResourceTransfer(const TouchObject<TEObject>& resource, const TouchObject<TESemaphore>& semaphore, uint64_t value)
{
	myPendingTransfers.emplace_back(resource, semaphore, value);
}

void Renderer::clearResourceTransfer(const TouchObject<TEObject>& resource)
{
	for (auto it = myPendingTransfers.begin(); it != myPendingTransfers.end(); )
	{
		if (it->resource == resource)
		{
			it = myPendingTransfers.erase(it);
		}
		else
		{
			it++;
		}
	}
}

size_t Renderer::alignedBufferSize(size_t size) const
{
	return size = ((size + myMinBufferAlignment - 1) / myMinBufferAlignment) * myMinBufferAlignment;
}

const TouchObject<TETexture>& Renderer::getOutputImage() const
{
	return myOutputImage;
}

void
Renderer::clearOutputs()
{
	myOutputImage.reset();
}

TouchObject<TEBuffer> Renderer::getHostBuffer(const void* src, size_t size)
{
	TouchObject<TEMutableHostBuffer> buffer = TouchObject<TEMutableHostBuffer>::make_take(TEMutableHostBufferCreate(size, nullptr, nullptr));

	void* dst = TEMutableHostBufferGetData(buffer);

	memcpy(dst, src, size);

	return buffer;
}

TouchObject<TEBuffer> Renderer::getDeviceBuffer(const void* src, size_t size)
{
	// default to use host buffers for everything, derived classes can override this
	return getHostBuffer(src, size);
}

void Renderer::addResourceTransfers(const TouchObject<TEInstance>& instance)
{
	for (const auto& next : myPendingTransfers)
	{
		TEInstanceAddResourceTransfer(instance, next.resource, next.semaphore, next.value);
	}
	myPendingTransfers.clear();
}

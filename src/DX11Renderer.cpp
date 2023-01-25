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
#include "DX11Renderer.h"
#include <TouchEngine/TouchEngine.h>
#include <TouchEngine/TED3D11.h>
#include <array>

DX11Renderer::DX11Renderer()
	: Renderer(), myDevice()
{
}


DX11Renderer::~DX11Renderer()
{
	
}

bool
DX11Renderer::setup(HWND window)
{
	Renderer::setup(window);
	HRESULT result = myDevice.createDeviceResources();
	if (SUCCEEDED(result))
	{
		// Create window resources with no depth-stencil buffer
		result = myDevice.createWindowResources(getWindow(), false);
	}
	if (SUCCEEDED(result))
	{
		myPixelShader = myDevice.loadPixelShader(L"TestPixelShader.cso");
		if (!myPixelShader)
		{
			result = EIO;
		}
	}
	if (SUCCEEDED(result))
	{
		const D3D11_INPUT_ELEMENT_DESC layoutDescription[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		myVertexShader = myDevice.loadVertexShader(L"TestVertexShader.cso", layoutDescription, ARRAYSIZE(layoutDescription));
		if (!myVertexShader.isValid())
		{
			result = EIO;
		}
	}
	if (SUCCEEDED(result))
	{
		if (TED3D11ContextCreate(myDevice.getDevice(), myContext.take()) != TEResultSuccess)
		{
			result = E_FAIL;
		}
	}
	return SUCCEEDED(result);
}

bool DX11Renderer::configure(TEInstance* instance, std::wstring& error)
{
	myReleaseToZero = TEInstanceRequiresKeyedMutexReleaseToZero(instance);
	return true;
}

bool DX11Renderer::doesTextureTransfer() const
{
	return true;
}

void
DX11Renderer::resize(int width, int height)
{
	Renderer::resize(width, height);
	myDevice.resize();
}

void
DX11Renderer::stop()
{
	myInputImages.clear();
	myOutputImages.clear();
	// Invalidate the vertex shader
	myVertexShader = DX11VertexShader();
	myDevice.stop();
}

bool
DX11Renderer::render()
{ 
	myDevice.setRenderTarget();
	myDevice.clear(myBackgroundColor[0], myBackgroundColor[1], myBackgroundColor[2], 1.0f);

	myDevice.setPixelShader(myPixelShader.Get());
	myDevice.setInputLayout(myVertexShader);
	myDevice.setVertexShader(myVertexShader);

	float scale = 1.0f / (max(myInputImages.size(), myOutputImages.size()) + 1.0f);
	drawImages(myInputImages, scale, -0.5f);
	drawImages(myOutputImages, scale, 0.5f);

	myDevice.present();
	return true;
}

void
DX11Renderer::addInputImage(const unsigned char* rgba, size_t bytesPerRow, int width, int height)
{
	DX11Texture texture = myDevice.loadTexture(rgba, int32_t(bytesPerRow), width, height);

	myInputImages.emplace_back(texture);
	myInputImages.back().setup(myDevice);
	Renderer::addInputImage(rgba, bytesPerRow, width, height);
}

bool DX11Renderer::getInputImage(size_t index, TouchObject<TETexture> & texture, TouchObject<TESemaphore> & semaphore, uint64_t & waitValue)
{
	if (inputDidChange(index))
	{
		auto& source = myInputImages[index];
		texture.take(TED3D11TextureCreate(source.getTexture().getTexture(), TETextureOriginTopLeft, kTETextureComponentMapIdentity, nullptr, nullptr));

		// The TED3D11Context handles sync for us, so we needn't set semaphore or waitValue

		markInputUnchanged(index);
		return true;
	}
	return false;
}

void
DX11Renderer::clearInputImages()
{
	myInputImages.clear();
}

void
DX11Renderer::addOutputImage()
{
	myOutputImages.emplace_back();
	myOutputImages.back().setup(myDevice);

	Renderer::addOutputImage();
}

bool DX11Renderer::releaseOutputImage(size_t index, TouchObject<TETexture>& texture, TouchObject<TESemaphore>& semaphore, uint64_t& waitValue)
{
	const auto& current = getOutputImage(index);
	if (current)
	{
		texture = current;
		// set from TEInstanceRequiresKeyedMutexReleaseToZero(), see the documentation for that function
		if (myReleaseToZero)
		{
			waitValue = 0;
		}
		else
		{
			waitValue = myOutputImages[index].getTexture().getLastAcquireValue();
			if (waitValue == UINT64_MAX)
				waitValue = 0;
			else
				waitValue++;
		}

		myOutputImages[index].getTexture().release(waitValue);
		return true;
	}
	return false;
}

void
DX11Renderer::setOutputImage(size_t index, const TouchObject<TETexture> &texture)
{
	bool success = false;
	if (texture && TETextureGetType(texture) == TETextureTypeD3DShared)
	{
		TouchObject<TED3D11Texture> created;
		if (TED3D11ContextGetTexture(myContext, static_cast<TED3DSharedTexture *>(texture.get()), created.take()) == TEResultSuccess)
		{
			DX11Texture tex(created);

			myOutputImages.at(index).update(tex);

			Renderer::setOutputImage(index, texture);

			success = true;
		}
	}

	if (!success)
	{
		myOutputImages.at(index).update(DX11Texture());
		Renderer::setOutputImage(index, nullptr);
	}
}

void DX11Renderer::acquireOutputImage(size_t index, TouchObject<TESemaphore>& semaphore, uint64_t& waitValue)
{
	myOutputImages[index].getTexture().acquire(waitValue);
}

void
DX11Renderer::clearOutputImages()
{
	myOutputImages.clear();

	Renderer::clearOutputImages();
}

void
DX11Renderer::drawImages(std::vector<DX11Image>& images, float scale, float xOffset)
{
	float numImages = (1.0f / scale) - 1.0f;
	float spacing = 1.0f / numImages;
	float yOffset = 1.0f - spacing;
	float ratio = static_cast<float>(myHeight) / myWidth;
	for (auto &image : images)
	{
		image.scale(scale * ratio, scale);
		image.position(xOffset, yOffset);
		image.draw(myDevice);
		yOffset -= spacing * 2;
	}
}

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
#include "DXUtility.h"
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

void
DX11Renderer::setup(HWND window)
{
	Renderer::setup(window);
	ThrowIfFailed(myDevice.createDeviceResources());

	// Create window resources with no depth-stencil buffer
	ThrowIfFailed(myDevice.createWindowResources(getWindow()));

	myPixelShader = myDevice.loadPixelShader(L"TestPixelShader.cso");
	if (!myPixelShader)
	{
		throw std::runtime_error("Couldn't create pixel shader.");
	}
	
	{
		const D3D11_INPUT_ELEMENT_DESC layoutDescription[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		myVertexShader = myDevice.loadVertexShader(L"TestVertexShader.cso", layoutDescription, ARRAYSIZE(layoutDescription));
		if (!myVertexShader.isValid())
		{
			throw std::runtime_error("Couldn't create vertex shader.");
		}
	}
	myBlendState = myDevice.createBlendState();
	
	if (!myOutputImage.setup(myDevice))
	{
		throw std::runtime_error("Couldn't create image resources.");
	}
	if (TED3D11ContextCreate(myDevice.getDevice(), myContext.take()) != TEResultSuccess)
	{
		throw std::runtime_error("Couldn't create TED3D11Context.");
	}
}

bool DX11Renderer::configure(TEInstance* instance, std::string& error)
{
	myReleaseToZero = TEInstanceRequiresKeyedMutexReleaseToZero(instance);
	return Renderer::configure(instance, error);
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
	myInputTexture = DX11Texture();
	myOutputImage = DX11Image();
	// Invalidate the vertex shader
	myVertexShader = DX11VertexShader();
	myDevice.stop();
}

bool
DX11Renderer::render()
{ 
	myDevice.setRenderTarget();
	myDevice.clear(myBackgroundColor);

	myDevice.setBlendState(myBlendState.Get());
	myDevice.setPixelShader(myPixelShader.Get());
	myDevice.setInputLayout(myVertexShader);
	myDevice.setVertexShader(myVertexShader);

	if (myOutputImage.getTexture().isValid())
	{
		myOutputImage.fit(static_cast<float>(myWidth), static_cast<float>(myHeight));
		myOutputImage.position(0.0f, 0.0f);
		myOutputImage.draw(myDevice);
	}

	myDevice.present();
	return true;
}

TouchObject<TETexture>	DX11Renderer::getTexture(const unsigned char* rgba, size_t bytesPerRow, int width, int height)
{
	myInputTexture = myDevice.loadTexture(rgba, int32_t(bytesPerRow), width, height);

	TouchObject<TETexture> texture;

	texture.take(TED3D11TextureCreate(myInputTexture.getTexture(), TETextureOriginTopLeft, kTETextureComponentMapIdentity, nullptr, nullptr));

	// The TED3D11Context handles sync for us, so we needn't add a resource transfer here

	return texture;
}

void
DX11Renderer::clearInputs()
{
	myInputTexture = DX11Texture();
	Renderer::clearInputs();
}

bool DX11Renderer::setOutputImage(const TouchObject<TETexture>& texture, const TouchObject<TESemaphore>& semaphore, uint64_t waitValue)
{
	setOutputImage(texture);

	// DXGI Keyed Mutexes will be used for sync, so semaphore will be null
	assert(!semaphore);
	myOutputImage.getTexture().acquire(waitValue);
	
	return true;
}

void DX11Renderer::setOutputImage(const TouchObject<TETexture>& texture)
{
	const auto& previous = getOutputImage();
	if (previous)
	{
		uint64_t waitValue;
		// set from TEInstanceRequiresKeyedMutexReleaseToZero(), see the documentation for that function
		if (myReleaseToZero)
		{
			waitValue = 0;
		}
		else
		{
			waitValue = myOutputImage.getTexture().getLastAcquireValue();
			if (waitValue == UINT64_MAX)
				waitValue = 0;
			else
				waitValue++;
		}

		myOutputImage.getTexture().release(waitValue);

		// DXGI Keyed Mutexes use the texture as the sync object, so `semaphore` is nullptr
		addResourceTransfer(previous, nullptr, waitValue);
	}
	if (texture && TETextureGetType(texture) == TETextureTypeD3DShared)
	{
		TouchObject<TED3D11Texture> created;
		if (TED3D11ContextGetTexture(myContext, static_cast<TED3DSharedTexture*>(texture.get()), created.take()) == TEResultSuccess)
		{
			DX11Texture tex(created);

			myOutputImage.update(tex);
		}
		else
		{
			myOutputImage.update(DX11Texture());
		}
	}
	else
	{
		myOutputImage.update(DX11Texture());
		Renderer::setOutputImage(nullptr);
	}
	Renderer::setOutputImage(texture);
}

void
DX11Renderer::clearOutputs()
{
	myOutputImage.update(DX11Texture());

	Renderer::clearOutputs();
}

const std::string& DX11Renderer::getDeviceName() const
{
	return myDevice.getDeviceName();
}

void
DX11Renderer::drawImage(DX11Image& image, float scale, float xOffset, float yOffset)
{
	image.scale(scale, scale);
	image.position(xOffset, yOffset);
	image.draw(myDevice);
}

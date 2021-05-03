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
#include "DirectXTexture.h"
#include "DirectXDevice.h"

DirectXTexture::DirectXTexture()
	: myTexture(nullptr), myTextureView(nullptr), mySampler(nullptr), myVFlipped(false)
{
}


DirectXTexture::DirectXTexture(DirectXTexture && o)
	: myTexture(o.myTexture), myTextureView(o.myTextureView), mySampler(o.mySampler), myVFlipped(o.myVFlipped)
{
	o.myTexture = nullptr;
	o.myTextureView = nullptr;
	o.mySampler = nullptr;
}

DirectXTexture & DirectXTexture::operator=(DirectXTexture && o)
{
	releaseResources();
	myTexture = o.myTexture;
	o.myTexture = nullptr;
	myTextureView = o.myTextureView;
	o.myTextureView = nullptr;
	mySampler = o.mySampler;
	o.mySampler = nullptr;
	myVFlipped = o.myVFlipped;
	return *this;
}

DirectXTexture::~DirectXTexture()
{
	releaseResources();
}

ID3D11Texture2D * DirectXTexture::getTexture() const
{
	return myTexture;
}

bool DirectXTexture::isValid() const
{
	if (myTexture && myTextureView && mySampler)
	{
		return true;
	}
	return false;
}

void DirectXTexture::setResourceAndSampler(ID3D11DeviceContext * context)
{
	context->PSSetShaderResources(0, 1, &myTextureView);
	context->PSSetSamplers(0, 1, &mySampler);
}

int DirectXTexture::getWidth() const
{
	if (myTexture)
	{
		D3D11_TEXTURE2D_DESC description;
		myTexture->GetDesc(&description);
		return description.Width;
	}
	return 0;
}

int DirectXTexture::getHeight() const
{
	if (myTexture)
	{
		D3D11_TEXTURE2D_DESC description;
		myTexture->GetDesc(&description);
		return description.Height;
	}
	return 0;
}

bool DirectXTexture::getFlipped() const
{
	return myVFlipped;
}

HRESULT DirectXTexture::createShaderResourceView(ID3D11Device *device, const D3D11_TEXTURE2D_DESC & description)
{
	D3D11_SHADER_RESOURCE_VIEW_DESC textureViewDescription;
	ZeroMemory(&textureViewDescription, sizeof(textureViewDescription));
	textureViewDescription.Format = description.Format;
	textureViewDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	textureViewDescription.Texture2D.MipLevels = description.MipLevels;
	textureViewDescription.Texture2D.MostDetailedMip = 0;
	return device->CreateShaderResourceView(myTexture, &textureViewDescription, &myTextureView);
}

HRESULT DirectXTexture::createSamplerState(ID3D11Device * device, const D3D11_TEXTURE2D_DESC & description)
{
	D3D11_SAMPLER_DESC samplerDescription;
	ZeroMemory(&samplerDescription, sizeof(samplerDescription));

	samplerDescription.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;

	samplerDescription.MaxAnisotropy = 0;
	samplerDescription.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDescription.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDescription.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDescription.MipLODBias = 0.0f;
	samplerDescription.MinLOD = 0;
	samplerDescription.MaxLOD = D3D11_FLOAT32_MAX;
	samplerDescription.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDescription.BorderColor[0] = 0.0f;
	samplerDescription.BorderColor[1] = 0.0f;
	samplerDescription.BorderColor[2] = 0.0f;
	samplerDescription.BorderColor[3] = 0.0f;

	return device->CreateSamplerState(&samplerDescription, &mySampler);
}

void DirectXTexture::releaseResources()
{
	if (myTexture)
	{
		myTexture->Release();
		myTexture = nullptr;
	}
	if (myTextureView)
	{
		myTextureView->Release();
		myTextureView = nullptr;
	}
	if (mySampler)
	{
		mySampler->Release();
		mySampler = nullptr;
	}
}

DirectXTexture::DirectXTexture(DirectXDevice &device, const unsigned char * src, int bytesPerRow, int width, int height, bool automips)
	: myTexture(nullptr), myTextureView(nullptr), mySampler(nullptr), myVFlipped(false)
{
	D3D11_SUBRESOURCE_DATA subresource = { 0 };
	subresource.pSysMem = src;
	subresource.SysMemPitch = bytesPerRow;
	subresource.SysMemSlicePitch = 0;

	D3D11_TEXTURE2D_DESC description = { 0 };
	description.Width = width;
	description.Height = height;
	description.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	description.Usage = D3D11_USAGE_DEFAULT;
	description.CPUAccessFlags = 0;
	description.MiscFlags = 0;
	description.MipLevels = automips ? 0 : 1;
	description.ArraySize = 1;
	description.SampleDesc.Count = 1;
	description.SampleDesc.Quality = 0;
	description.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	if (automips)
	{
		description.BindFlags |= D3D11_BIND_RENDER_TARGET;
		description.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
	}

	HRESULT result = device.getDevice()->CreateTexture2D(&description, automips ? nullptr : &subresource, &myTexture);

	if (SUCCEEDED(result))
	{
		myTexture->GetDesc(&description);
		result = createShaderResourceView(device.getDevice(), description);
	}

	if (SUCCEEDED(result) && automips)
	{
		device.updateSubresource(myTexture, src, bytesPerRow, bytesPerRow * height);
		device.generateMips(myTextureView);
	}

	if (SUCCEEDED(result))
	{
		result = createSamplerState(device.getDevice(), description);
	}

	if (!SUCCEEDED(result))
	{
		releaseResources();
	}
}

DirectXTexture::DirectXTexture(ID3D11Texture2D * texture, bool flipped)
	: myTexture(texture), myTextureView(nullptr), mySampler(nullptr), myVFlipped(flipped)
{
	texture->AddRef();

	D3D11_TEXTURE2D_DESC description = { 0 };
	myTexture->GetDesc(&description);

	ID3D11Device *device;
	texture->GetDevice(&device);

	HRESULT result = createShaderResourceView(device, description);

	if (SUCCEEDED(result))
	{
		result = createSamplerState(device, description);
	}

	if (!SUCCEEDED(result))
	{
		releaseResources();
	}
}

DirectXTexture::DirectXTexture(const DirectXTexture & o)
	: myTexture(o.myTexture), myTextureView(o.myTextureView), mySampler(o.mySampler), myVFlipped(o.myVFlipped)
{
	if (myTexture)
	{
		myTexture->AddRef();
	}
	if (myTextureView)
	{
		myTextureView->AddRef();
	}
	if (mySampler)
	{
		mySampler->AddRef();
	}
}

DirectXTexture & DirectXTexture::operator=(const DirectXTexture & o)
{
	if (&o != this)
	{
		releaseResources();
		myTexture = o.myTexture;
		if (myTexture)
		{
			myTexture->AddRef();
		}
		myTextureView = o.myTextureView;
		if (myTextureView)
		{
			myTextureView->AddRef();
		}
		mySampler = o.mySampler;
		if (mySampler)
		{
			mySampler->AddRef();
		}
		myVFlipped = o.myVFlipped;
	}
	return *this;
}

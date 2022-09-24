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
#include "DX11Texture.h"
#include "DX11Device.h"
#include <TouchEngine/TED3D11.h>

DX11Texture::DX11Texture()
{
}

ID3D11Texture2D * DX11Texture::getTexture() const
{
	return myTexture.Get();
}

bool
DX11Texture::isValid() const
{
	if (myTexture && myTextureView && mySampler)
	{
		return true;
	}
	return false;
}

void
DX11Texture::setResourceAndSampler(ID3D11DeviceContext * context)
{
	context->PSSetShaderResources(0, 1, myTextureView.GetAddressOf());
	context->PSSetSamplers(0, 1, mySampler.GetAddressOf());
}

int
DX11Texture::getWidth() const
{
	if (myTexture)
	{
		D3D11_TEXTURE2D_DESC description;
		myTexture->GetDesc(&description);
		return description.Width;
	}
	return 0;
}

int
DX11Texture::getHeight() const
{
	if (myTexture)
	{
		D3D11_TEXTURE2D_DESC description;
		myTexture->GetDesc(&description);
		return description.Height;
	}
	return 0;
}

bool
DX11Texture::getFlipped() const
{
	return myVFlipped;
}

void DX11Texture::acquire(uint64_t value)
{
	myKeyedMutex->AcquireSync(value, INFINITE);
	myLastAcquireValue = value;
}

void DX11Texture::release(uint64_t value)
{
	myKeyedMutex->ReleaseSync(value);
}

HRESULT
DX11Texture::createShaderResourceView(ID3D11Device* device, const D3D11_TEXTURE2D_DESC & description)
{
	D3D11_SHADER_RESOURCE_VIEW_DESC textureViewDescription;
	ZeroMemory(&textureViewDescription, sizeof(textureViewDescription));
	textureViewDescription.Format = description.Format;
	textureViewDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	textureViewDescription.Texture2D.MipLevels = description.MipLevels;
	textureViewDescription.Texture2D.MostDetailedMip = 0;
	return device->CreateShaderResourceView(myTexture.Get(), &textureViewDescription, &myTextureView);
}

HRESULT
DX11Texture::createSamplerState(ID3D11Device* device, const D3D11_TEXTURE2D_DESC & description)
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

void
DX11Texture::releaseResources()
{
	myTexture.Reset();
	myKeyedMutex.Reset();
	myTextureView.Reset();
	mySampler.Reset();
	mySource.reset();
}

DX11Texture::DX11Texture(DX11Device &device, const unsigned char * src, int bytesPerRow, int width, int height, bool automips)
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
		device.updateSubresource(myTexture.Get(), src, bytesPerRow, bytesPerRow * (size_t)height);
		device.generateMips(myTextureView.Get());
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

DX11Texture::DX11Texture(const TouchObject<TED3D11Texture> &source)
	: mySource(source), myTexture(TED3D11TextureGetTexture(source)),
		myVFlipped(TETextureGetOrigin(source) != TETextureOriginTopLeft)
{
	myTexture->QueryInterface(IID_PPV_ARGS(&myKeyedMutex));

	D3D11_TEXTURE2D_DESC description = { 0 };
	myTexture->GetDesc(&description);

	ID3D11Device *device;
	myTexture->GetDevice(&device);

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

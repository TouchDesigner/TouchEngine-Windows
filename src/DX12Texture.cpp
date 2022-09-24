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
#include "DX12Texture.h"
#include "DX12Utility.h"
#include <TouchEngine/TED3D12.h>

using Microsoft::WRL::ComPtr;

DX12Texture::DX12Texture()
{
}

DX12Texture::DX12Texture(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, const unsigned char* src, size_t bytesPerRow, int width, int height, bool genMips)
	: myWidth(width), myHeight(height), myDevice(device)
{
	D3D12_RESOURCE_DESC textureDesc = {};

	textureDesc.MipLevels = 1;
	textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	textureDesc.DepthOrArraySize = 1;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

	ThrowIfFailed(device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_SHARED,
		&textureDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&myResource)));

	{
		const UINT64 uploadBufferSize = GetRequiredIntermediateSize(myResource.Get(), 0, 1);

		device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&myTextureUploadHeap));
	}

	{
		D3D12_SUBRESOURCE_DATA textureData = {};
		textureData.pData = src;
		textureData.RowPitch = bytesPerRow;
		textureData.SlicePitch = textureData.RowPitch * height;

		UpdateSubresources(commandList, myResource.Get(), myTextureUploadHeap.Get(), 0, 0, 1, &textureData);
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(myResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	}

	setupSRV(textureDesc);

	{
		HANDLE handle;
		ThrowIfFailed(device->CreateSharedHandle(myResource.Get(), nullptr, GENERIC_ALL, nullptr, &handle));
		
		myTETexture.take(TED3DSharedTextureCreate(handle, TED3DHandleTypeD3D12ResourceNT, textureDesc.Format, textureDesc.Width, textureDesc.Height, TETextureOriginTopLeft, kTETextureComponentMapIdentity, nullptr, nullptr));

		// TouchEngine duplicates it for its own use
		CloseHandle(handle);
	}
}

DX12Texture::DX12Texture(ID3D12Device* device, TED3DSharedTexture *texture)
	: myFlipped(TETextureGetOrigin(texture) == TETextureOriginBottomLeft), myDevice(device)
{
	HANDLE h = TED3DSharedTextureGetHandle(texture);
	TED3DHandleType type = TED3DSharedTextureGetHandleType(texture);
	if (type == TED3DHandleTypeD3D12ResourceNT)
	{
		HRESULT hr = device->OpenSharedHandle(h, IID_PPV_ARGS(&myResource));
		ThrowIfFailed(hr);
	}
	else
	{
		throw std::runtime_error("Unexpected TED3DHandleType");
	}
	if (myResource.Get())
	{
		D3D12_RESOURCE_DESC resourceDesc = myResource->GetDesc();
		myWidth = static_cast<int>(resourceDesc.Width);
		myHeight = resourceDesc.Height;
		setupSRV(resourceDesc);
	}
}

void DX12Texture::uploadDidComplete()
{
	myTextureUploadHeap.Reset();
}

ID3D12Resource* DX12Texture::getResource() const
{
	return myResource.Get();
}

bool DX12Texture::isValid() const
{
	return myResource.Get() != nullptr;
}

void DX12Texture::setupSRV(D3D12_RESOURCE_DESC& textureDesc)
{
	{
		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
		srvHeapDesc.NumDescriptors = 1;
		srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		myDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mySRVHeap));
	}

	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = textureDesc.Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		myDevice->CreateShaderResourceView(myResource.Get(), &srvDesc, mySRVHeap->GetCPUDescriptorHandleForHeapStart());
	}
}

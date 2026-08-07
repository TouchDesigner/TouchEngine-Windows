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
#include "DX12CommandList.h"
#include "DXUtility.h"
#include <TouchEngine/TED3D12.h>

using Microsoft::WRL::ComPtr;

DX12Texture::DX12Texture()
{
}

DX12Texture::DX12Texture(ID3D12Device* device, DX12CommandList& commandList, const unsigned char* src, size_t bytesPerRow, int width, int height)
	: myWidth(width), myHeight(height)
{
	D3D12_RESOURCE_DESC textureDesc = {};

	textureDesc.MipLevels = 1;
	textureDesc.Format = Format;
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	textureDesc.DepthOrArraySize = 1;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	CD3DX12_HEAP_PROPERTIES heapDefault(D3D12_HEAP_TYPE_DEFAULT);
	ThrowIfFailed(device->CreateCommittedResource(&heapDefault,
		D3D12_HEAP_FLAG_SHARED,
		&textureDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&myResource)));

	Microsoft::WRL::ComPtr<ID3D12Resource>	textureUploadHeap;

	{
		const UINT64 uploadBufferSize = GetRequiredIntermediateSize(myResource.Get(), 0, 1);

		CD3DX12_HEAP_PROPERTIES heapUpload(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC buffer(CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize));
		device->CreateCommittedResource(&heapUpload,
			D3D12_HEAP_FLAG_NONE,
			&buffer,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&textureUploadHeap));
	}

	{
		D3D12_SUBRESOURCE_DATA textureData = {};
		textureData.pData = src;
		textureData.RowPitch = bytesPerRow;
		textureData.SlicePitch = textureData.RowPitch * height;

		UpdateSubresources(commandList, myResource.Get(), textureUploadHeap.Get(), 0, 0, 1, &textureData);

		commandList.barrier(myResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON);
		commandList.usingResource(textureUploadHeap);
		commandList.usingResource(myResource);
	}
}

DX12Texture::DX12Texture(ID3D12Device* device, HANDLE h)
{
	ThrowIfFailed(device->OpenSharedHandle(h, IID_PPV_ARGS(&myResource)));
	if (myResource.Get())
	{
		D3D12_RESOURCE_DESC resourceDesc = myResource->GetDesc();
		myWidth = static_cast<int>(resourceDesc.Width);
		myHeight = resourceDesc.Height;
	}
}

DX12Texture::operator ID3D12Resource* () const
{
	return myResource.Get();
}

bool DX12Texture::isValid() const
{
	return myResource.Get() != nullptr;
}

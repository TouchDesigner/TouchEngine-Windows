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
#include "DX12Image.h"
#include "DXUtility.h"
#include "DX12CommandList.h"

DX12Image::DX12Image()
{

}

DX12Image::DX12Image(ID3D12Device* device)
{
    setup(device);
}

void DX12Image::update(ID3D12Device* device, const DX12Texture& texture)
{
    if (myTexture.getFlipped() != texture.getFlipped() || myTexture.getWidth() != texture.getWidth() || myTexture.getHeight() != texture.getHeight())
    {
        changed = true;
    }
    myTexture = texture;
    width = static_cast<float>(myTexture.getWidth());
    height = static_cast<float>(myTexture.getHeight());
    mySRVHeap.Reset();
    if (myTexture.isValid())
    {
        setupSRV(device);
    }
}

void DX12Image::draw(DX12CommandList& commandList)
{
    if (myTexture.isValid())
    {
        if (changed)
        {
            bool flipped = myTexture.getFlipped();

            BasicVertex rectangleVertices[] =
            {
                { { (-1.0f * scaleX) + x, (-1.0f * scaleY) + y, 0.0f }, { 0.0f, flipped ? 0.0f : 1.0f } },
                { { (-1.0f * scaleX) + x, ( 1.0f * scaleY) + y, 0.0f }, { 0.0f, flipped ? 1.0f : 0.0f } },
                { {  (1.0f * scaleX) + x, (-1.0f * scaleY) + y, 0.0f }, { 1.0f, flipped ? 0.0f : 1.0f } },
                { {  (1.0f * scaleX) + x, ( 1.0f * scaleY) + y, 0.0f }, { 1.0f, flipped ? 1.0f : 0.0f } }
            };

            UINT8* pVertexDataBegin = nullptr;
            CD3DX12_RANGE readRange(0, 0);
            ThrowIfFailed(myVertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
            memcpy(pVertexDataBegin, rectangleVertices, sizeof(rectangleVertices));
            myVertexBuffer->Unmap(0, nullptr);

            changed = false;
        }
        ID3D12DescriptorHeap* heaps[] = { mySRVHeap.Get() };

        commandList.usingResource(myVertexBuffer);
        commandList.usingResource(myTexture);

        commandList.get()->SetDescriptorHeaps(_countof(heaps), heaps);

        commandList.get()->SetGraphicsRootDescriptorTable(0, mySRVHeap->GetGPUDescriptorHandleForHeapStart());

        commandList.get()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
        commandList.get()->IASetVertexBuffers(0, 1, &myVertexBufferView);
        commandList.get()->DrawInstanced(4, 1, 0, 0);
    }
}

void DX12Image::setup(ID3D12Device *device)
{
    const UINT vertexBufferSize = sizeof(BasicVertex) * 4;

    CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC buffer(CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize));
    ThrowIfFailed(device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &buffer,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&myVertexBuffer)));

    // Initialize the vertex buffer view.
    myVertexBufferView.BufferLocation = myVertexBuffer->GetGPUVirtualAddress();
    myVertexBufferView.StrideInBytes = sizeof(BasicVertex);
    myVertexBufferView.SizeInBytes = vertexBufferSize;
}

void DX12Image::setupSRV(ID3D12Device* device)
{
    {
        D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
        srvHeapDesc.NumDescriptors = 1;
        srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mySRVHeap));
    }

    {
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = DX12Texture::Format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;
        device->CreateShaderResourceView(myTexture, &srvDesc, mySRVHeap->GetCPUDescriptorHandleForHeapStart());
    }
}

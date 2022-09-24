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
#include "DX12Utility.h"

DX12Image::DX12Image()
{

}

DX12Image::DX12Image(ID3D12Device* device)
{
    setup(device);
}

DX12Image::DX12Image(DX12Texture& texture)
	: Drawable(0.0f, 0.0f, static_cast<float>(texture.getWidth()), static_cast<float>(texture.getHeight())), myTexture(texture)
{
    setup(texture.getDevice());
}

void DX12Image::update(DX12Texture& texture)
{
    if (myTexture.getFlipped() != texture.getFlipped() || myTexture.getWidth() != texture.getWidth() || myTexture.getHeight() != texture.getHeight())
    {
        myMatrixDirty = true;
    }
    myTexture = texture;
    width = static_cast<float>(myTexture.getWidth());
    height = static_cast<float>(myTexture.getHeight());
}

void DX12Image::position(float newx, float newy)
{
    if (newx != x || newy != y)
    {
        x = newx;
        y = newy;
        myMatrixDirty = true;
    }
}

void DX12Image::scale(float scaleX, float scaleY)
{
    if (myScaleX != scaleX || myScaleY != scaleY)
    {
        myScaleX = scaleX;
        myScaleY = scaleY;
        myMatrixDirty = true;
    }
}

void DX12Image::draw(ID3D12GraphicsCommandList* commandList)
{
    if (myTexture.isValid())
    {
        if (myMatrixDirty)
        {
            bool flipped = myTexture.getFlipped();

            float ratio = width == 0.0f ? 1.0f : height / width;

            BasicVertex rectangleVertices[] =
            {
                { { (-1.0f * myScaleX) + x, (-1.0f * myScaleY * ratio) + y, 0.0f }, { 0.0f, flipped ? 0.0f : 1.0f } },
                { { (-1.0f * myScaleX) + x, ( 1.0f * myScaleY * ratio) + y, 0.0f }, { 0.0f, flipped ? 1.0f : 0.0f } },
                { {  (1.0f * myScaleX) + x, (-1.0f * myScaleY * ratio) + y, 0.0f }, { 1.0f, flipped ? 0.0f : 1.0f } },
                { {  (1.0f * myScaleX) + x, ( 1.0f * myScaleY * ratio) + y, 0.0f }, { 1.0f, flipped ? 1.0f : 0.0f } }
            };

            UINT8* pVertexDataBegin;
            CD3DX12_RANGE readRange(0, 0);
            ThrowIfFailed(myVertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
            memcpy(pVertexDataBegin, rectangleVertices, sizeof(rectangleVertices));
            myVertexBuffer->Unmap(0, nullptr);

            myMatrixDirty = false;
        }
        ID3D12DescriptorHeap* heaps[] = { myTexture.getSRVHeap() };

        commandList->SetDescriptorHeaps(_countof(heaps), heaps);

        commandList->SetGraphicsRootDescriptorTable(0, myTexture.getSRVHeap()->GetGPUDescriptorHandleForHeapStart());

        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
        commandList->IASetVertexBuffers(0, 1, &myVertexBufferView);
        commandList->DrawInstanced(4, 1, 0, 0);
    }
}

void DX12Image::setup(ID3D12Device *device)
{
    const UINT vertexBufferSize = sizeof(BasicVertex) * 4;

    ThrowIfFailed(device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&myVertexBuffer)));

    // Initialize the vertex buffer view.
    myVertexBufferView.BufferLocation = myVertexBuffer->GetGPUVirtualAddress();
    myVertexBufferView.StrideInBytes = sizeof(BasicVertex);
    myVertexBufferView.SizeInBytes = vertexBufferSize;
}

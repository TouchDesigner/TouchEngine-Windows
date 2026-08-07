/*
* 1. The shared file or redistribution must retain the information set out
* above and this list of conditions.
* 2. Derivative's name (Derivative Inc.) or its trademarks may not be used
* to endorse or promote products derived from this file without specific
* prior written permission from Derivative.
*/

#include "stdafx.h"
#include "DX12UploadBuffer.h"

DX12UploadBuffer::DX12UploadBuffer(ID3D12Device* device, size_t size)
	: DX12Buffer(device, D3D12_HEAP_TYPE_UPLOAD, D3D12_HEAP_FLAG_NONE, size)
{
	// No CPU reads will be done from the resource.
	CD3DX12_RANGE readRange(0, 0);
	myBuffer->Map(0, &readRange, &myBufferData);
}

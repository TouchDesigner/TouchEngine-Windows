/*
* 1. The shared file or redistribution must retain the information set out
* above and this list of conditions.
* 2. Derivative's name (Derivative Inc.) or its trademarks may not be used
* to endorse or promote products derived from this file without specific
* prior written permission from Derivative.
*/

#pragma once

#include "DX12Buffer.h"
class DX12UploadBuffer :
    public DX12Buffer
{
public:
	DX12UploadBuffer() = default;
	DX12UploadBuffer(ID3D12Device* device, size_t size);
	void* data() const
	{
		return myBufferData;
	}
private:
	void* myBufferData = nullptr;
};


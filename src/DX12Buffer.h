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

#pragma once

#include <d3d12.h>

class DX12Buffer
{
public:
	DX12Buffer() = default;
	DX12Buffer(ID3D12Device* device, D3D12_HEAP_TYPE type, D3D12_HEAP_FLAGS flags, size_t size);
	size_t getSize() const
	{
		return mySize;
	}
	size_t getRequiredUploadSize() const;
	operator ID3D12Resource* () const
	{
		return myBuffer.Get();
	}
protected:
	Microsoft::WRL::ComPtr<ID3D12Resource> myBuffer;
private:
	size_t mySize = 0;
};

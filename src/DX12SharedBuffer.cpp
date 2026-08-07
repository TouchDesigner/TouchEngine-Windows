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
#include "DX12SharedBuffer.h"
#include "DXUtility.h"

DX12SharedBuffer::DX12SharedBuffer(ID3D12Device* device, size_t size)
: DX12Buffer(device, D3D12_HEAP_TYPE_DEFAULT, D3D12_HEAP_FLAG_SHARED, size)
{
	ThrowIfFailed(device->CreateSharedHandle(myBuffer.Get(), nullptr, GENERIC_ALL, nullptr, &myHandle));
}

DX12SharedBuffer::~DX12SharedBuffer()
{
	if (myHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(myHandle);
	}
}

DX12SharedBuffer& DX12SharedBuffer::operator=(DX12SharedBuffer&& o) noexcept
{
	if (&o != this)
	{
		DX12Buffer::operator=(o);
		std::swap(o.myHandle, myHandle);
	}
	return *this;
}

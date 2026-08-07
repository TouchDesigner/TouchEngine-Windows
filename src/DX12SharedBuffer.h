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
#include "DX12Buffer.h"

class DX12SharedBuffer :
    public DX12Buffer
{
public:
    DX12SharedBuffer() = default;
    DX12SharedBuffer(ID3D12Device* device, size_t size);
    ~DX12SharedBuffer();
    DX12SharedBuffer(const DX12SharedBuffer&) = delete;
    DX12SharedBuffer& operator=(const DX12SharedBuffer&) = delete;
    DX12SharedBuffer& operator=(DX12SharedBuffer&& o) noexcept;
    HANDLE getSharedHandle() const
    {
        return myHandle;
    }
private:
    HANDLE myHandle = INVALID_HANDLE_VALUE;
};


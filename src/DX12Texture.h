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
#include <TouchEngine/TouchObject.h>
#include <set>

class DX12CommandList;

class DX12Texture
{
public:
	DX12Texture();
	DX12Texture(ID3D12Device *device, DX12CommandList &commandList, const unsigned char* src, size_t bytesPerRow, int width, int height);
	DX12Texture(ID3D12Device* device, HANDLE h);

	operator ID3D12Resource* () const;
	bool				isValid() const;
	constexpr int		getWidth() const
	{
		return myWidth;
	}
	constexpr int		getHeight() const
	{
		return myHeight;
	}
	constexpr bool		getFlipped() const
	{
		return myFlipped;
	}

	static constexpr DXGI_FORMAT Format = DXGI_FORMAT_B8G8R8A8_UNORM;
private:
	int myWidth = 0;
	int myHeight = 0;
	bool myFlipped = false;
	Microsoft::WRL::ComPtr<ID3D12Resource>	myResource;
};

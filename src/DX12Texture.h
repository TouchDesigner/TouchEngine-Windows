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

class DX12Texture
{
public:
	DX12Texture();
	DX12Texture(ID3D12Device *device, ID3D12GraphicsCommandList *commandList, const unsigned char* src, size_t bytesPerRow, int width, int height, bool genMips = false);
	DX12Texture(ID3D12Device* device, TED3DSharedTexture *texture);

	void				uploadDidComplete();

	ID3D12Resource*		getResource() const;
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

	ID3D12DescriptorHeap* getSRVHeap() const
	{
		return mySRVHeap.Get();
	}
	ID3D12Device* getDevice() const
	{
		return myDevice.Get();
	}

	TED3DSharedTexture* getTETexture() const
	{
		return myTETexture;
	}
private:
	void setupSRV(D3D12_RESOURCE_DESC &desc);
	int myWidth = 0;
	int myHeight = 0;
	bool myFlipped = false;
	Microsoft::WRL::ComPtr<ID3D12Device> myDevice;
	Microsoft::WRL::ComPtr<ID3D12Resource>	myResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> myTextureUploadHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mySRVHeap;
	TouchObject<TED3DSharedTexture> myTETexture;
};

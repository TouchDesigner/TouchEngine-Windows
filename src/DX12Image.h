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

#include "Drawable.h"
#include "DX12Texture.h"
#include <DirectXMath.h>

class DX12Image :
	public Drawable
{
public:
	DX12Image();
	DX12Image(ID3D12Device* device);
	DX12Image(const DX12Texture& texture);
	constexpr DX12Texture& getTexture()
	{
		return myTexture;
	}
	void update(DX12Texture& texture);
	void position(float newx, float newy);
	void scale(float scaleX, float scaleY);
	void draw(ID3D12GraphicsCommandList* commandList);
private:
	struct BasicVertex
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT2 tex;
	};
	void											setup(ID3D12Device *device);
	DX12Texture										myTexture;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>	mySRVHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource>			myVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW						myVertexBufferView{0, 0, 0};
	bool											myMatrixDirty{ true };
	float											myScaleX{ 1.0 };
	float											myScaleY{ 1.0 };
};

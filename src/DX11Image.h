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
#include "DX11Texture.h"
#include "DX11VertexShader.h"
#include <DirectXMath.h>

class DX11Device;

class DX11Image :
	public Drawable
{
public:
	DX11Image();
	DX11Image(DX11Texture& texture);

	bool				setup(DX11Device& device);
	void				draw(DX11Device& device);
	void				position(float x, float y);
	void				scale(float scaleX, float scaleY);
	DX11Texture &		getTexture();
	void				update(const DX11Texture &texture);
private:
	struct BasicVertex
	{
		DirectX::XMFLOAT2 pos;
		DirectX::XMFLOAT2 tex;
	};
	struct ConstantBuffer
	{
		DirectX::XMMATRIX matrix;
		DirectX::XMINT4 flip; // XM type for easy alignment, only .x used
	};
	DX11Texture								myTexture;
	Microsoft::WRL::ComPtr<ID3D11Buffer>	myVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>	myIndexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>	myConstantBuffer;
	bool									myMatrixDirty{ true };
	float									myScaleX{ 1.0 };
	float									myScaleY{ 1.0 };
};


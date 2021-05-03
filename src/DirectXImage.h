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
#include "DirectXTexture.h"
#include "VertexShader.h"
#include <DirectXMath.h>

class DirectXDevice;

class DirectXImage :
	public Drawable
{
public:
	DirectXImage();
	DirectXImage(DirectXTexture& texture);
	DirectXImage(DirectXImage&& other);
	~DirectXImage();

	bool				setup(DirectXDevice& device);
	void				draw(DirectXDevice& device);
	void				position(float x, float y);
	void				scale(float scaleX, float scaleY);
	ID3D11Texture2D*	getTexture() const;
	void				update(DirectXTexture &texture);
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
	DirectXTexture	myTexture;
	ID3D11Buffer*	myVertexBuffer;
	ID3D11Buffer*	myIndexBuffer;
	ID3D11Buffer*	myConstantBuffer;
	bool			myMatrixDirty;
	float			myScaleX{ 1.0 };
	float			myScaleY{ 1.0 };
};


/* Shared Use License: This file is owned by Derivative Inc. (Derivative) and
 * can only be used, and/or modified for use, in conjunction with
 * Derivative's TouchDesigner software, and only if you are a licensee who has
 * accepted Derivative's TouchDesigner license or assignment agreement (which
 * also govern the use of this file).  You may share a modified version of this
 * file with another authorized licensee of Derivative's TouchDesigner software.
 * Otherwise, no redistribution or sharing of this file, with or without
 * modification, is permitted.
 *
 * TouchEngine
 *
 * Copyright © 2021 Derivative. All rights reserved.
 *
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


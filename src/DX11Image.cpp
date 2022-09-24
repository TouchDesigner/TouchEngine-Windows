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
#include "DX11Image.h"
#include "DX11Device.h"

DX11Image::DX11Image()
	: Drawable()
{
}

DX11Image::DX11Image(DX11Texture & texture)
	: Drawable(0.0f, 0.0f, static_cast<float>(texture.getWidth()), static_cast<float>(texture.getHeight())),
	myTexture(texture)
{
}

bool
DX11Image::setup(DX11Device& device)
{
	BasicVertex rectangleVertices[] =
	{
		{ DirectX::XMFLOAT2(-1.0f, -1.0f),  DirectX::XMFLOAT2(0.0f, 1.0f) },
		{ DirectX::XMFLOAT2(-1.0f,  1.0f),  DirectX::XMFLOAT2(0.0f, 0.0f) },
		{ DirectX::XMFLOAT2(1.0f, -1.0f),  DirectX::XMFLOAT2(1.0f, 1.0f) },
		{ DirectX::XMFLOAT2(1.0f,  1.0f),  DirectX::XMFLOAT2(1.0f, 0.0f) }
	};

	myVertexBuffer = device.loadVertexBuffer(rectangleVertices, ARRAYSIZE(rectangleVertices));

	unsigned short rectangleIndices[] =
	{
		0, 1, 2, 3
	};

	myIndexBuffer = device.loadIndexBuffer(rectangleIndices, ARRAYSIZE(rectangleIndices));

	myConstantBuffer = device.loadConstantBuffer<ConstantBuffer>();

	if (myVertexBuffer && myIndexBuffer && myConstantBuffer)
	{
		return true;
	}
	return false;
}

void
DX11Image::draw(DX11Device &device)
{
	if (myTexture.isValid())
	{
		if (myMatrixDirty)
		{
			ConstantBuffer cbuffer;

			float ratio = height / width;
			cbuffer.matrix = DirectX::XMMatrixIdentity();
			cbuffer.matrix *= DirectX::XMMatrixScaling(myScaleX, myScaleY * ratio, 1.0f);
			cbuffer.matrix *= DirectX::XMMatrixTranslation(x, y, 0);
			cbuffer.matrix = DirectX::XMMatrixTranspose(cbuffer.matrix);
			cbuffer.flip.x = myTexture.getFlipped();

			device.updateSubresource(myConstantBuffer.Get(), &cbuffer);
			
			myMatrixDirty = false;
		}

		device.setVertexBuffer<BasicVertex>(myVertexBuffer.Get());
		device.setIndexBuffer(myIndexBuffer.Get());
		device.setTriangleStripTopology();
		device.setConstantBuffer(myConstantBuffer.Get());
		device.setShaderResourceAndSampler(myTexture);
		device.drawIndexed(4);
	}
}

void
DX11Image::position(float newx, float newy)
{
	if (x != newx || y != newy)
	{
		x = newx;
		y = newy;
		myMatrixDirty = true;
	}
}

void
DX11Image::scale(float scaleX, float scaleY)
{
	if (myScaleX != scaleX || myScaleY != scaleY)
	{
		myScaleX = scaleX;
		myScaleY = scaleY;
		myMatrixDirty = true;
	}
}

DX11Texture &
DX11Image::getTexture()
{
	return myTexture;
}

void
DX11Image::update(DX11Texture & texture)
{
	if (myTexture.getFlipped() != texture.getFlipped() || myTexture.getWidth() != texture.getWidth() || myTexture.getHeight() != texture.getHeight())
	{
		myMatrixDirty = true;
	}
	myTexture = texture;
	width = (float)myTexture.getWidth();
	height = (float)myTexture.getHeight();
}

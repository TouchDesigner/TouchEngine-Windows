#include "stdafx.h"
#include "DirectXImage.h"
#include "DirectXDevice.h"

DirectXImage::DirectXImage()
    : Drawable(), myVertexBuffer(nullptr), myIndexBuffer(nullptr), myConstantBuffer(nullptr), myMatrixDirty(true)
{
}

DirectXImage::DirectXImage(DirectXTexture & texture)
    : Drawable(0.0f, 0.0f, static_cast<float>(texture.getWidth()), static_cast<float>(texture.getHeight())),
    myTexture(texture), myVertexBuffer(nullptr), myIndexBuffer(nullptr), myConstantBuffer(nullptr), myMatrixDirty(true)
{
}

DirectXImage::DirectXImage(DirectXImage && other)
    : Drawable(other), myTexture(other.myTexture),
    myVertexBuffer(other.myVertexBuffer), myIndexBuffer(other.myIndexBuffer), myConstantBuffer(other.myConstantBuffer),
    myMatrixDirty(true), myScaleX(other.myScaleX), myScaleY(other.myScaleY)
{
    other.myVertexBuffer = nullptr;
    other.myIndexBuffer = nullptr;
    other.myConstantBuffer = nullptr;
}


DirectXImage::~DirectXImage()
{
    if (myVertexBuffer)
    {
        myVertexBuffer->Release();
    }
    if (myIndexBuffer)
    {
        myIndexBuffer->Release();
    }
    if (myConstantBuffer)
    {
        myConstantBuffer->Release();
    }
}

bool DirectXImage::setup(DirectXDevice & device)
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

void DirectXImage::draw(DirectXDevice &device)
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

            device.updateSubresource(myConstantBuffer, &cbuffer);
            
            myMatrixDirty = false;
        }

        device.setVertexBuffer<BasicVertex>(myVertexBuffer);
        device.setIndexBuffer(myIndexBuffer);
        device.setTriangleStripTopology();
        device.setConstantBuffer(myConstantBuffer);
        device.setShaderResourceAndSampler(myTexture);
        device.drawIndexed(4);
    }
}

void DirectXImage::position(float newx, float newy)
{
    if (x != newx || y != newy)
    {
        x = newx;
        y = newy;
        myMatrixDirty = true;
    }
}

void DirectXImage::scale(float scaleX, float scaleY)
{
    if (myScaleX != scaleX || myScaleY != scaleY)
    {
        myScaleX = scaleX;
        myScaleY = scaleY;
        myMatrixDirty = true;
    }
}

ID3D11Texture2D * DirectXImage::getTexture() const
{
    return myTexture.getTexture();
}

void DirectXImage::update(DirectXTexture & texture)
{
    if (myTexture.getFlipped() != texture.getFlipped())
    {
        myMatrixDirty = true;
    }
    myTexture = texture;
    width = (float)myTexture.getWidth();
    height = (float)myTexture.getHeight();
}

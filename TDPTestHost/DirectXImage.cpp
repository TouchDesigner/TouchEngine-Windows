#include "stdafx.h"
#include "DirectXImage.h"
#include "DirectXDevice.h"

DirectXImage::DirectXImage()
    : Drawable(), myPixelShader(nullptr), myVertexBuffer(nullptr), myIndexBuffer(nullptr), myConstantBuffer(nullptr), myMatrixDirty(true), myScale(1.0f)
{
}

DirectXImage::DirectXImage(DirectXTexture & texture)
    : Drawable(0.0f, 0.0f, static_cast<float>(texture.getWidth()), static_cast<float>(texture.getHeight())),
    myTexture(texture), myPixelShader(nullptr), myVertexBuffer(nullptr), myIndexBuffer(nullptr), myConstantBuffer(nullptr), myMatrixDirty(true), myScale(1.0)
{
}

DirectXImage::DirectXImage(DirectXImage && other)
    : myTexture(other.myTexture), myVertexShader(std::move(other.myVertexShader)),
    myPixelShader(other.myPixelShader), myVertexBuffer(other.myVertexBuffer), myIndexBuffer(other.myIndexBuffer), myConstantBuffer(other.myConstantBuffer),
    myMatrixDirty(true), myScale(1.0)
{
    other.myPixelShader = nullptr;
    other.myVertexBuffer = nullptr;
    other.myIndexBuffer = nullptr;
    other.myConstantBuffer = nullptr;
}


DirectXImage::~DirectXImage()
{
    if (myPixelShader)
    {
        myPixelShader->Release();
    }
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
    const D3D11_INPUT_ELEMENT_DESC layoutDescription[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    myVertexShader = device.loadVertexShader(L"TDPTestVertexShader.cso", layoutDescription, ARRAYSIZE(layoutDescription));
    myPixelShader = device.loadPixelShader(L"TDPTestPixelShader.cso");

    BasicVertex rectangleVertices[] =
    {
        { DirectX::XMFLOAT2(-1.0f, -1.0f),  DirectX::XMFLOAT2(0.0f, 0.0f) },
        { DirectX::XMFLOAT2(-1.0f,  1.0f),  DirectX::XMFLOAT2(0.0f, 1.0f) },
        { DirectX::XMFLOAT2(1.0f, -1.0f),  DirectX::XMFLOAT2(1.0f, 0.0f) },
        { DirectX::XMFLOAT2(1.0f,  1.0f),  DirectX::XMFLOAT2(1.0f, 1.0f) }
    };

    myVertexBuffer = device.loadVertexBuffer(rectangleVertices, ARRAYSIZE(rectangleVertices));

    unsigned short rectangleIndices[] =
    {
        0, 1, 2, 2, 1, 3
    };

    myIndexBuffer = device.loadIndexBuffer(rectangleIndices, ARRAYSIZE(rectangleIndices));

    myConstantBuffer = device.loadConstantBuffer<ConstantBuffer>();

    if (myVertexShader.isValid() && myPixelShader && myVertexBuffer && myIndexBuffer && myConstantBuffer)
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

            cbuffer.matrix = DirectX::XMMatrixIdentity();
            cbuffer.matrix *= DirectX::XMMatrixScaling(myScale, myScale, 1.0f);
            cbuffer.matrix *= DirectX::XMMatrixTranslation(x, y, 0);
            cbuffer.matrix = DirectX::XMMatrixTranspose(cbuffer.matrix);

            device.updateSubresource(myConstantBuffer, &cbuffer);
            
            myMatrixDirty = false;
        }

        device.setInputLayout(myVertexShader);
        device.setVertexBuffer<BasicVertex>(myVertexBuffer);
        device.setIndexBuffer(myIndexBuffer);
        device.setTriangleListTopology();
        device.setVertexShader(myVertexShader);
        device.setConstantBuffer(myConstantBuffer);
        device.setPixelShader(myPixelShader);
        device.setShaderResourceAndSampler(myTexture);
        device.drawIndexed(6);
    }
    else
    {
        int i = 0;
    }
}

void DirectXImage::position(float newx, float newy)
{
    x = newx;
    y = newy;
    myMatrixDirty = true;
}

void DirectXImage::scale(float scale)
{
    myScale = scale;
    myMatrixDirty = true;
}

ID3D11Texture2D * DirectXImage::getTexture() const
{
    return myTexture.getTexture();
}

void DirectXImage::update(DirectXTexture & texture)
{
    myTexture = texture;
    width = myTexture.getWidth();
    height = myTexture.getHeight();
}

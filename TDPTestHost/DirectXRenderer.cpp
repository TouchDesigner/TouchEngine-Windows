#include "stdafx.h"
#include "DirectXRenderer.h"
#include "Device.h"
#include <array>

DirectXRenderer::DirectXRenderer(Device &device)
    : myDevice(device), myVertexShader(), myPixelShader(nullptr), myVertexBuffer(nullptr), myIndexBuffer(nullptr)
{
}


DirectXRenderer::~DirectXRenderer()
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
}

bool DirectXRenderer::setup()
{
    const D3D11_INPUT_ELEMENT_DESC layoutDescription[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    myVertexShader = myDevice.loadVertexShader(L"TDPTestVertexShader.cso", layoutDescription, ARRAYSIZE(layoutDescription));
    myPixelShader = myDevice.loadPixelShader(L"TDPTestPixelShader.cso");

    BasicVertex rectangleVertices[] =
    {
        { DirectX::XMFLOAT2(-0.5f, -0.5f),  DirectX::XMFLOAT2(0.0f, 0.0f) },
        { DirectX::XMFLOAT2(-0.5f,  0.5f),  DirectX::XMFLOAT2(0.0f, 1.0f) },
        { DirectX::XMFLOAT2( 0.5f, -0.5f),  DirectX::XMFLOAT2(1.0f, 0.0f) },
        { DirectX::XMFLOAT2( 0.5f,  0.5f),  DirectX::XMFLOAT2(1.0f, 1.0f) }
    };

    myVertexBuffer = myDevice.loadVertexBuffer(rectangleVertices, ARRAYSIZE(rectangleVertices));

    unsigned short rectangleIndices[] =
    {
        0, 1, 2, 2, 1, 3
    };

    myIndexBuffer = myDevice.loadIndexBuffer(rectangleIndices, ARRAYSIZE(rectangleIndices));

    std::array<unsigned char, 256 * 256 * 4> tex;

    for (int y = 0; y < 256; y++)
    {
        for (int x = 0; x < 256; x++)
        {
            tex[(y * 256 * 4) + (x * 4) + 0] = x;
            tex[(y * 256 * 4) + (x * 4) + 1] = 40;
            tex[(y * 256 * 4) + (x * 4) + 2] = y;
            tex[(y * 256 * 4) + (x * 4) + 3] = 255;
        }
    }

    myTexture = myDevice.loadTexture(tex.data(), 256 * 4, 256, 256);

    if (myVertexShader.isValid() && myPixelShader && myVertexBuffer && myIndexBuffer && myTexture.isValid())
    {
        return true;
    }
    return false;
}

bool DirectXRenderer::render()
{
    myDevice.setRenderTarget();
    myDevice.clear(myBackgroundColor[0], myBackgroundColor[1], myBackgroundColor[2], 1.0f);
    myDevice.setInputLayout(myVertexShader);
    myDevice.setVertexBuffer<BasicVertex>(myVertexBuffer);
    myDevice.setIndexBuffer(myIndexBuffer);
    myDevice.setTriangleListTopology();
    myDevice.setVertexShader(myVertexShader);
    myDevice.setPixelShader(myPixelShader);
    myDevice.setShaderResourceAndSampler(myTexture);
    myDevice.drawIndexed(6);
    myDevice.present();
    return true;
}

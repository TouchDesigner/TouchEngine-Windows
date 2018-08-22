#include "stdafx.h"
#include "DirectXTexture.h"


DirectXTexture::DirectXTexture()
    : myTexture(nullptr), myTextureView(nullptr), mySampler(nullptr)
{
}


DirectXTexture::DirectXTexture(DirectXTexture && o)
    : myTexture(o.myTexture), myTextureView(o.myTextureView), mySampler(o.mySampler), myWidth(o.myWidth), myHeight(o.myHeight)
{
    o.myTexture = nullptr;
    o.myTextureView = nullptr;
    o.mySampler = nullptr;
}

DirectXTexture & DirectXTexture::operator=(DirectXTexture && o)
{
    releaseResources();
    myTexture = o.myTexture;
    o.myTexture = nullptr;
    myTextureView = o.myTextureView;
    o.myTextureView = nullptr;
    mySampler = o.mySampler;
    o.mySampler = nullptr;
    myWidth = o.myWidth;
    myHeight = o.myHeight;
    return *this;
}

DirectXTexture::~DirectXTexture()
{
    releaseResources();
}

ID3D11Texture2D * DirectXTexture::getTexture() const
{
    return myTexture;
}

bool DirectXTexture::isValid() const
{
    if (myTexture && myTextureView && mySampler)
    {
        return true;
    }
    return false;
}

void DirectXTexture::setResourceAndSampler(ID3D11DeviceContext * context)
{
    context->PSSetShaderResources(0, 1, &myTextureView);
    context->PSSetSamplers(0, 1, &mySampler);
}

void DirectXTexture::releaseResources()
{
    if (myTexture)
    {
        myTexture->Release();
        myTexture = nullptr;
    }
    if (myTextureView)
    {
        myTextureView->Release();
        myTextureView = nullptr;
    }
    if (mySampler)
    {
        mySampler->Release();
        mySampler = nullptr;
    }
}

DirectXTexture::DirectXTexture(ID3D11Device *device, const unsigned char * src, int bytesPerRow, int width, int height)
{
    D3D11_SUBRESOURCE_DATA subresource = { 0 };
    subresource.pSysMem = src;
    subresource.SysMemPitch = bytesPerRow;
    subresource.SysMemSlicePitch = 0;

    D3D11_TEXTURE2D_DESC description = { 0 };
    description.Width = width;
    description.Height = height;
    description.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    description.Usage = D3D11_USAGE_DEFAULT;
    description.CPUAccessFlags = 0;
    description.MiscFlags = 0;
    description.MipLevels = 1;
    description.ArraySize = 1;
    description.SampleDesc.Count = 1;
    description.SampleDesc.Quality = 0;
    description.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    HRESULT result = device->CreateTexture2D(&description, &subresource, &myTexture);

    if (SUCCEEDED(result))
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC textureViewDescription;
        ZeroMemory(&textureViewDescription, sizeof(textureViewDescription));
        textureViewDescription.Format = description.Format;
        textureViewDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        textureViewDescription.Texture2D.MipLevels = description.MipLevels;
        textureViewDescription.Texture2D.MostDetailedMip = 0;
        result = device->CreateShaderResourceView(myTexture, &textureViewDescription, &myTextureView);
    }

    if (SUCCEEDED(result))
    {
        D3D11_SAMPLER_DESC samplerDescription;
        ZeroMemory(&samplerDescription, sizeof(samplerDescription));

        samplerDescription.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;

        samplerDescription.MaxAnisotropy = 0;
        samplerDescription.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDescription.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDescription.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDescription.MipLODBias = 0.0f;
        samplerDescription.MinLOD = 0;
        samplerDescription.MaxLOD = D3D11_FLOAT32_MAX;
        samplerDescription.ComparisonFunc = D3D11_COMPARISON_NEVER;
        samplerDescription.BorderColor[0] = 0.0f;
        samplerDescription.BorderColor[1] = 0.0f;
        samplerDescription.BorderColor[2] = 0.0f;
        samplerDescription.BorderColor[3] = 0.0f;

        result = device->CreateSamplerState(&samplerDescription, &mySampler);
    }

    if (SUCCEEDED(result))
    {
        myWidth = width;
        myHeight = height;
    }
    else
    {
        releaseResources();
    }
}


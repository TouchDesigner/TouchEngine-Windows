#pragma once
#include "Renderer.h"
#include "VertexShader.h"
#include "DirectXTexture.h"
#include <DirectXMath.h>

class Device;

class DirectXRenderer :
    public Renderer
{
public:
    DirectXRenderer(Device &device);
    virtual ~DirectXRenderer();
    virtual bool setup() override;
    virtual bool render() override;
    ID3D11Texture2D *getTexture() const {
        return myTexture.getTexture();
    }
private:
    struct BasicVertex
    {
        DirectX::XMFLOAT2 pos;
        DirectX::XMFLOAT2 tex;
    };
    Device &myDevice;
    VertexShader myVertexShader;
    ID3D11PixelShader *myPixelShader;
    ID3D11Buffer *myVertexBuffer;
    ID3D11Buffer *myIndexBuffer;
    DirectXTexture myTexture;
};


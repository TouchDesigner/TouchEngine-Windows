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
    DirectXImage(DirectXTexture &texture);
    DirectXImage(DirectXImage &&other);
    ~DirectXImage();
    bool setup(DirectXDevice &device);
    void draw(DirectXDevice &device);
    void position(float x, float y);
    void scale(float scale);
    ID3D11Texture2D *getTexture() const;
    void update(DirectXTexture &texture);
private:
    struct BasicVertex
    {
        DirectX::XMFLOAT2 pos;
        DirectX::XMFLOAT2 tex;
    };
    struct ConstantBuffer
    {
        DirectX::XMMATRIX matrix;
    };
    DirectXTexture myTexture;
    VertexShader myVertexShader;
    ID3D11PixelShader *myPixelShader;
    ID3D11Buffer *myVertexBuffer;
    ID3D11Buffer *myIndexBuffer;
    ID3D11Buffer *myConstantBuffer;
    bool myMatrixDirty;
    float myScale;
};

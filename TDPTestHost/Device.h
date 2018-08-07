#pragma once

#include <string>
#include "VertexShader.h"
#include "DirectXTexture.h"

class Device
{
public:
    Device();
    ~Device();
    HRESULT createDeviceResources();
    HRESULT createWindowResources(HWND window, bool depth);
    VertexShader loadVertexShader(const std::wstring &file, const D3D11_INPUT_ELEMENT_DESC *layout, int count);
    ID3D11PixelShader *loadPixelShader(const std::wstring &file);
    template <class T>
    ID3D11Buffer * loadVertexBuffer(T * vertices, int count)
    {
        return loadBuffer(sizeof(T) * count, D3D11_BIND_VERTEX_BUFFER, vertices);
    }
    ID3D11Buffer *loadIndexBuffer(unsigned short *indices, int count);
    DirectXTexture loadTexture(const unsigned char *src, int bytesPerRow, int width, int height);
    void setRenderTarget();
    void clear(float r, float g, float b, float a);
    void present();
    void setInputLayout(VertexShader &shader);
    template <class T>
    void setVertexBuffer(ID3D11Buffer * buffer)
    {
        UINT stride = sizeof(T);
        UINT offset = 0;
        myDeviceContext->IASetVertexBuffers(0, 1, &buffer, &stride, &offset);
    }
    void setIndexBuffer(ID3D11Buffer *buffer);
    void setTriangleListTopology();
    void setVertexShader(VertexShader &shader);
    void setPixelShader(ID3D11PixelShader *shader);
    void setShaderResourceAndSampler(DirectXTexture &texture);
    void drawIndexed(int count);
private:
    std::wstring getResourcePath() const;
    HRESULT configureBackBuffer(bool depth);
    HRESULT releaseBackBuffer();
    ID3D11Buffer *loadBuffer(unsigned int size, D3D11_BIND_FLAG flags, const void *data);
    ID3D11Device *myDevice;
    ID3D11DeviceContext *myDeviceContext;
    D3D_FEATURE_LEVEL myFeatureLevel;
    IDXGISwapChain *mySwapChain;
    ID3D11Texture2D *myBackBuffer;
    ID3D11RenderTargetView *myRenderTarget;
    ID3D11Texture2D *myDepthStencil;
    ID3D11DepthStencilView *myDepthStencilView;

    D3D11_TEXTURE2D_DESC myBackBufferDescription;
    D3D11_VIEWPORT myViewport;
};


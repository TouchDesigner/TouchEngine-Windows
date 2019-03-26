#pragma once

#include <string>
#include "VertexShader.h"
#include "DirectXTexture.h"

class DirectXDevice
{
public:
    DirectXDevice();
    ~DirectXDevice();
    DirectXDevice(const DirectXDevice &o) = delete;
    DirectXDevice &operator=(const DirectXDevice &o) = delete;
    HRESULT createDeviceResources();
    HRESULT createWindowResources(HWND window, bool depth);
	HRESULT resize();
    VertexShader loadVertexShader(const std::wstring &file, const D3D11_INPUT_ELEMENT_DESC *layout, int count);
    ID3D11PixelShader *loadPixelShader(const std::wstring &file);
    template <class T>
    ID3D11Buffer * loadVertexBuffer(T * vertices, int count)
    {
        return loadBuffer(sizeof(T) * count, D3D11_BIND_VERTEX_BUFFER, vertices);
    }
    ID3D11Buffer *loadIndexBuffer(unsigned short *indices, int count);
    template <class T>
    ID3D11Buffer *loadConstantBuffer()
    {
        return loadBuffer(sizeof(T), D3D11_BIND_CONSTANT_BUFFER, nullptr);
    }
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
    void setTriangleStripTopology();
    void setVertexShader(VertexShader &shader);
    void setPixelShader(ID3D11PixelShader *shader);
    void setShaderResourceAndSampler(DirectXTexture &texture);
    void updateSubresource(ID3D11Resource *resource, const void *data);
    void setConstantBuffer(ID3D11Buffer *buffer);
    void drawIndexed(int count);
    ID3D11Device *getDevice() const { return myDevice; }
    void stop();
private:
    std::wstring getResourcePath() const;
    HRESULT configureBackBuffer(bool depth);
    void releaseEverything();
    HRESULT releaseBackBuffer();
    ID3D11Buffer *loadBuffer(unsigned int size, D3D11_BIND_FLAG flags, const void *data);
    ID3D11Device *myDevice = nullptr;
    ID3D11DeviceContext *myDeviceContext = nullptr;
    D3D_FEATURE_LEVEL myFeatureLevel;
    IDXGISwapChain *mySwapChain = nullptr;
    ID3D11Texture2D *myBackBuffer = nullptr;
    ID3D11RenderTargetView *myRenderTarget = nullptr;
    ID3D11Texture2D *myDepthStencil = nullptr;
    ID3D11DepthStencilView *myDepthStencilView = nullptr;

    D3D11_TEXTURE2D_DESC myBackBufferDescription;
    D3D11_VIEWPORT myViewport;
};


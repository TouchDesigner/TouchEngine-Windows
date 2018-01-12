#pragma once
class Device
{
public:
    Device();
    ~Device();
    HRESULT createDeviceResources();
    HRESULT createWindowResources(HWND window);
private:
    HRESULT configureBackBuffer();
    HRESULT releaseBackBuffer();
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


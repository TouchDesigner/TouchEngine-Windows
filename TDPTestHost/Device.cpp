#include "stdafx.h"
#include "Device.h"


Device::Device()
{
}


Device::~Device()
{
    releaseBackBuffer();
    if (mySwapChain)
    {
        mySwapChain->Release();
    }
    if (myDeviceContext)
    {
        myDeviceContext->Release();
    }
    if (myDevice)
    {
        myDevice->Release();
    }
}

HRESULT Device::createDeviceResources()
{
    D3D_FEATURE_LEVEL levels[] = {
        D3D_FEATURE_LEVEL_9_1,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_11_1
    };

    UINT deviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(DEBUG) || defined(_DEBUG)
    deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    HRESULT result = D3D11CreateDevice(nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        0,
        deviceFlags,
        levels,
        ARRAYSIZE(levels),
        D3D11_SDK_VERSION,
        &myDevice,
        &myFeatureLevel,
        &myDeviceContext);

    return result;
}

HRESULT Device::createWindowResources(HWND window)
{
    DXGI_SWAP_CHAIN_DESC desc;
    ZeroMemory(&desc, sizeof(DXGI_SWAP_CHAIN_DESC));

    desc.Windowed = TRUE;
    desc.BufferCount = 2;
    desc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    desc.OutputWindow = window;

    IDXGIAdapter *adapter = nullptr;
    IDXGIFactory *factory = nullptr;

    IDXGIDevice *dxgiDevice = nullptr;

    HRESULT result = myDevice->QueryInterface(__uuidof(IDXGIDevice), (void **)&dxgiDevice);

    if (SUCCEEDED(result))
    {
        result = dxgiDevice->GetAdapter(&adapter);

        if (SUCCEEDED(result))
        {
            adapter->GetParent(IID_PPV_ARGS(&factory));

            result = factory->CreateSwapChain(myDevice, &desc, &mySwapChain);
        }

        if (adapter)
        {
            adapter->Release();
        }
        if (factory)
        {
            factory->Release();
        }
        if (dxgiDevice)
        {
            dxgiDevice->Release();
        }
    }

    if (SUCCEEDED(result))
    {
        result = configureBackBuffer();
    }

    return result;
}

HRESULT Device::configureBackBuffer()
{
    HRESULT result = mySwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&myBackBuffer);

    if (SUCCEEDED(result))
    {
        result = myDevice->CreateRenderTargetView(myBackBuffer, nullptr, &myRenderTarget);
    }
    if (SUCCEEDED(result))
    {
        myBackBuffer->GetDesc(&myBackBufferDescription);
        CD3D11_TEXTURE2D_DESC depthStencilDesc(DXGI_FORMAT_D24_UNORM_S8_UINT,
            static_cast<UINT>(myBackBufferDescription.Width),
            static_cast<UINT>(myBackBufferDescription.Height),
            1,
            1,
            D3D11_BIND_DEPTH_STENCIL);

        myDevice->CreateTexture2D(&depthStencilDesc, nullptr, &myDepthStencil);

        CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);

        myDevice->CreateDepthStencilView(myDepthStencil, &depthStencilViewDesc, &myDepthStencilView);

        ZeroMemory(&myViewport, sizeof(D3D11_VIEWPORT));
        myViewport.Height = (float)myBackBufferDescription.Height;
        myViewport.Width = (float)myBackBufferDescription.Width;
        myViewport.MinDepth = 0;
        myViewport.MaxDepth = 1;

        myDeviceContext->RSSetViewports(1, &myViewport);

        return result;
    }
    return result;
}

HRESULT Device::releaseBackBuffer()
{
    if (myRenderTarget)
    {
        myRenderTarget->Release();
        myRenderTarget = nullptr;
    }
    if (myBackBuffer)
    {
        myBackBuffer->Release();
        myBackBuffer = nullptr;
    }
    if (myDepthStencilView)
    {
        myDepthStencilView->Release();
        myDepthStencilView = nullptr;
    }
    if (myDepthStencil)
    {
        myDepthStencil->Release();
        myDepthStencil = nullptr;
    }
    myDeviceContext->Flush();
    return S_OK;
}

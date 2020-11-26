#include "stdafx.h"
#include "DirectXDevice.h"
#include "FileReader.h"

DirectXDevice::DirectXDevice()
{
}


DirectXDevice::~DirectXDevice()
{
    releaseEverything();
}

HRESULT DirectXDevice::createDeviceResources()
{
    D3D_FEATURE_LEVEL levels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0
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

HRESULT DirectXDevice::createWindowResources(HWND window, bool depth)
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
        result = configureBackBuffer(depth);
    }

    return result;
}

HRESULT DirectXDevice::resize()
{
	bool hasDepth = myDepthStencil != nullptr;

	myDeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
	
	releaseBackBuffer();

	HRESULT result = mySwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
	if (SUCCEEDED(result))
	{
		result = configureBackBuffer(hasDepth);
	}
	return result;
}

VertexShader DirectXDevice::loadVertexShader(const std::wstring & file, const D3D11_INPUT_ELEMENT_DESC *layoutDescription, int count)
{
    VertexShader out;
    HRESULT result = S_OK;

    std::wstring path = getResourcePath();

    if (path.empty())
    {
        result = EBADF;
    }


    FileReader reader(path + file);
    std::vector<unsigned char> compiled;

    if (!reader.read(compiled))
    {
        result = EBADF;
    }

    ID3D11VertexShader *shader = nullptr;

    if (SUCCEEDED(result))
    {
        result = myDevice->CreateVertexShader(compiled.data(), compiled.size(), nullptr, &shader);
    }
    
    ID3D11InputLayout *layout = nullptr;

    if (SUCCEEDED(result))
    {
        result = myDevice->CreateInputLayout(layoutDescription, count, compiled.data(), compiled.size(), &layout);
    }
    if (SUCCEEDED(result))
    {
        out = VertexShader(shader, layout);
    }
    if (shader)
    {
        shader->Release();
    }
    if (layout)
    {
        layout->Release();
    }
    return out;
}

ID3D11PixelShader * DirectXDevice::loadPixelShader(const std::wstring & file)
{
    HRESULT result = S_OK;

    std::wstring path = getResourcePath();

    if (path.empty())
    {
        result = EBADF;
    }


    FileReader reader(path + file);
    std::vector<unsigned char> compiled;

    if (!reader.read(compiled))
    {
        result = EBADF;
    }

    ID3D11PixelShader *shader = nullptr;

    if (SUCCEEDED(result))
    {
        result = myDevice->CreatePixelShader(compiled.data(), compiled.size(), nullptr, &shader);
    }

    if (SUCCEEDED(result))
    {
        return shader;
    }
    return nullptr;
}

ID3D11Buffer * DirectXDevice::loadIndexBuffer(unsigned short * indices, int count)
{
    return loadBuffer(sizeof(unsigned short) * count, D3D11_BIND_INDEX_BUFFER, indices);
}

DirectXTexture DirectXDevice::loadTexture(const unsigned char * src, int bytesPerRow, int width, int height)
{
    return DirectXTexture(*this, src, bytesPerRow, width, height, true);
}

void DirectXDevice::setRenderTarget()
{
    myDeviceContext->OMSetRenderTargets(1, &myRenderTarget, myDepthStencilView);
}

void DirectXDevice::clear(float r, float g, float b, float a)
{
    const float color[4] = { r, g, b, a };
    myDeviceContext->ClearRenderTargetView(myRenderTarget, color);
}

void DirectXDevice::present()
{
    mySwapChain->Present(1, 0);
}

void DirectXDevice::setInputLayout(VertexShader & shader)
{
    shader.setInputLayout(myDeviceContext);
}

void DirectXDevice::setIndexBuffer(ID3D11Buffer * buffer)
{
    myDeviceContext->IASetIndexBuffer(buffer, DXGI_FORMAT_R16_UINT, 0);
}

void DirectXDevice::setTriangleStripTopology()
{
    myDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}

void DirectXDevice::setVertexShader(VertexShader & shader)
{
    shader.setShader(myDeviceContext);
}

void DirectXDevice::setPixelShader(ID3D11PixelShader * shader)
{
    myDeviceContext->PSSetShader(shader, nullptr, 0);
}

void DirectXDevice::setShaderResourceAndSampler(DirectXTexture & texture)
{
    texture.setResourceAndSampler(myDeviceContext);
}

void DirectXDevice::updateSubresource(ID3D11Resource * resource, const void * data)
{
    myDeviceContext->UpdateSubresource(resource, 0, nullptr, data, 0, 0);
}

void DirectXDevice::updateSubresource(ID3D11Resource * resource, const void * data, size_t bytesPerRow, size_t bytesPerImage)
{
	myDeviceContext->UpdateSubresource(resource, 0, nullptr, data, static_cast<UINT>(bytesPerRow), bytesPerImage);
}

void DirectXDevice::generateMips(ID3D11ShaderResourceView *view)
{
	myDeviceContext->GenerateMips(view);
}

void DirectXDevice::setConstantBuffer(ID3D11Buffer * buffer)
{
    myDeviceContext->VSSetConstantBuffers(0, 1, &buffer);
}

void DirectXDevice::drawIndexed(int count)
{
    myDeviceContext->DrawIndexed(count, 0, 0);
}

void DirectXDevice::stop()
{
    myDeviceContext->ClearState();
    releaseEverything();
}

std::wstring DirectXDevice::getResourcePath() const
{
    WCHAR buffer[MAX_PATH];
    std::wstring prefix;
    if (GetModuleFileName(nullptr, buffer, MAX_PATH))
    {
        WCHAR drive[_MAX_DRIVE];
        WCHAR dir[_MAX_DIR];
        if (_wsplitpath_s(buffer, drive, _MAX_DRIVE, dir, _MAX_DIR, nullptr, 0, nullptr, 0) == 0)
        {
            return std::wstring(drive) + dir;
        }
    }
    return std::wstring();
}

HRESULT DirectXDevice::configureBackBuffer(bool depth)
{
    HRESULT result = mySwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&myBackBuffer);

    if (SUCCEEDED(result))
    {
        result = myDevice->CreateRenderTargetView(myBackBuffer, nullptr, &myRenderTarget);
    }
    if (SUCCEEDED(result))
    {
        myBackBuffer->GetDesc(&myBackBufferDescription);

        if (depth)
        {
            CD3D11_TEXTURE2D_DESC depthStencilDesc(DXGI_FORMAT_D24_UNORM_S8_UINT,
                static_cast<UINT>(myBackBufferDescription.Width),
                static_cast<UINT>(myBackBufferDescription.Height),
                1,
                1,
                D3D11_BIND_DEPTH_STENCIL);

            myDevice->CreateTexture2D(&depthStencilDesc, nullptr, &myDepthStencil);

            CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);

            myDevice->CreateDepthStencilView(myDepthStencil, &depthStencilViewDesc, &myDepthStencilView);
        }

        ZeroMemory(&myViewport, sizeof(D3D11_VIEWPORT));
        myViewport.Height = (float)myBackBufferDescription.Height;
        myViewport.Width = (float)myBackBufferDescription.Width;
        myViewport.MinDepth = 0;
        myViewport.MaxDepth = 1;

        myDeviceContext->RSSetViewports(1, &myViewport);
    }
    return result;
}

void DirectXDevice::releaseEverything()
{
    releaseBackBuffer();
    if (mySwapChain)
    {
        mySwapChain->Release();
        mySwapChain = nullptr;
    }
    if (myDeviceContext)
    {
        myDeviceContext->Release();
        myDeviceContext = nullptr;
    }
    if (myDevice)
    {
        //ID3D11Debug *debug;
        //myDevice->QueryInterface(__uuidof(ID3D11Debug), (LPVOID *)&debug);
        myDevice->Release();
        myDevice = nullptr;
        //debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
        //debug->Release();
    }
}

HRESULT DirectXDevice::releaseBackBuffer()
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
    if (myDeviceContext)
    {
        myDeviceContext->Flush();
    }
    return S_OK;
}

ID3D11Buffer * DirectXDevice::loadBuffer(unsigned int size, D3D11_BIND_FLAG flags, const void *data)
{
    D3D11_BUFFER_DESC bufferDescription = { 0 };
    bufferDescription.ByteWidth = size;
    bufferDescription.Usage = D3D11_USAGE_DEFAULT;
    bufferDescription.BindFlags = flags;
    bufferDescription.CPUAccessFlags = 0;
    bufferDescription.MiscFlags = 0;
    bufferDescription.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA vertexBufferData;
    vertexBufferData.pSysMem = data;
    vertexBufferData.SysMemPitch = 0;
    vertexBufferData.SysMemSlicePitch = 0;

    ID3D11Buffer *buffer;

    HRESULT result = myDevice->CreateBuffer(&bufferDescription, data ? &vertexBufferData : nullptr, &buffer);

    if (SUCCEEDED(result))
    {
        return buffer;
    }
    return nullptr;
}

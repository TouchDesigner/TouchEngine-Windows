/* Shared Use License: This file is owned by Derivative Inc. (Derivative)
* and can only be used, and/or modified for use, in conjunction with
* Derivative's TouchDesigner software, and only if you are a licensee who has
* accepted Derivative's TouchDesigner license or assignment agreement
* (which also govern the use of this file). You may share or redistribute
* a modified version of this file provided the following conditions are met:
*
* 1. The shared file or redistribution must retain the information set out
* above and this list of conditions.
* 2. Derivative's name (Derivative Inc.) or its trademarks may not be used
* to endorse or promote products derived from this file without specific
* prior written permission from Derivative.
*/

#include "stdafx.h"
#include "DX11Device.h"
#include "FileReader.h"
#include "DXGIUtility.h"

using Microsoft::WRL::ComPtr;

DX11Device::DX11Device()
{
}

HRESULT
DX11Device::createDeviceResources()
{
	ComPtr<IDXGIFactory4> factory;
	HRESULT result = CreateDXGIFactory2(0, IID_PPV_ARGS(&factory));

	if (FAILED(result))
	{
		return result;
	}

	DXGIUtility utility;

	utility.setDX11();

	std::wstring description;
	ComPtr<IDXGIAdapter1> adapter = utility.getHardwareAdapter(factory.Get(), description, true);

	if (adapter.Get() == nullptr)
	{
		return E_FAIL;
	}

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

	result = D3D11CreateDevice(adapter.Get(),
		D3D_DRIVER_TYPE_UNKNOWN,
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

HRESULT
DX11Device::createWindowResources(HWND window, bool depth)
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

	ComPtr<IDXGIAdapter> adapter;
	ComPtr<IDXGIFactory> factory;
	ComPtr<IDXGIDevice> dxgiDevice;

	HRESULT result = myDevice->QueryInterface(IID_PPV_ARGS(&dxgiDevice));

	if (SUCCEEDED(result))
	{
		result = dxgiDevice->GetAdapter(&adapter);

		if (SUCCEEDED(result))
		{
			adapter->GetParent(IID_PPV_ARGS(&factory));

			result = factory->CreateSwapChain(myDevice.Get(), &desc, &mySwapChain);
		}
	}

	if (SUCCEEDED(result))
	{
		result = configureBackBuffer(depth);
	}

	return result;
}

HRESULT
DX11Device::resize()
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

DX11VertexShader
DX11Device::loadVertexShader(const std::wstring & file, const D3D11_INPUT_ELEMENT_DESC *layoutDescription, int count)
{
	DX11VertexShader out;
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

	ComPtr<ID3D11VertexShader> shader;

	if (SUCCEEDED(result))
	{
		result = myDevice->CreateVertexShader(compiled.data(), compiled.size(), nullptr, &shader);
	}
	
	ComPtr<ID3D11InputLayout> layout;

	if (SUCCEEDED(result))
	{
		result = myDevice->CreateInputLayout(layoutDescription, count, compiled.data(), compiled.size(), &layout);
	}
	if (SUCCEEDED(result))
	{
		out = DX11VertexShader(shader.Get(), layout.Get());
	}
	return out;
}

ComPtr<ID3D11PixelShader>
DX11Device::loadPixelShader(const std::wstring & file)
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

	ComPtr<ID3D11PixelShader> shader;

	if (SUCCEEDED(result))
	{
		result = myDevice->CreatePixelShader(compiled.data(), compiled.size(), nullptr, &shader);
	}

	if (SUCCEEDED(result))
	{
		return shader;
	}
	return ComPtr<ID3D11PixelShader>();
}

ComPtr<ID3D11Buffer>
DX11Device::loadIndexBuffer(unsigned short * indices, int count)
{
	return loadBuffer(sizeof(unsigned short) * count, D3D11_BIND_INDEX_BUFFER, indices);
}

DX11Texture
DX11Device::loadTexture(const unsigned char * src, int bytesPerRow, int width, int height)
{
	return DX11Texture(*this, src, bytesPerRow, width, height, true);
}

void
DX11Device::setRenderTarget()
{
	myDeviceContext->OMSetRenderTargets(1, myRenderTarget.GetAddressOf(), myDepthStencilView.Get());
}

void
DX11Device::clear(float r, float g, float b, float a)
{
	const float color[4] = { r, g, b, a };
	myDeviceContext->ClearRenderTargetView(myRenderTarget.Get(), color);
}

void
DX11Device::present()
{
	mySwapChain->Present(1, 0);
}

void
DX11Device::setInputLayout(DX11VertexShader & shader)
{
	shader.setInputLayout(myDeviceContext.Get());
}

void
DX11Device::setIndexBuffer(ID3D11Buffer * buffer)
{
	myDeviceContext->IASetIndexBuffer(buffer, DXGI_FORMAT_R16_UINT, 0);
}

void
DX11Device::setTriangleStripTopology()
{
	myDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}

void
DX11Device::setVertexShader(DX11VertexShader & shader)
{
	shader.setShader(myDeviceContext.Get());
}

void
DX11Device::setPixelShader(ID3D11PixelShader * shader)
{
	myDeviceContext->PSSetShader(shader, nullptr, 0);
}

void
DX11Device::setShaderResourceAndSampler(DX11Texture & texture)
{
	texture.setResourceAndSampler(myDeviceContext.Get());
}

void
DX11Device::updateSubresource(ID3D11Resource * resource, const void * data)
{
	myDeviceContext->UpdateSubresource(resource, 0, nullptr, data, 0, 0);
}

void
DX11Device::updateSubresource(ID3D11Resource * resource, const void * data, size_t bytesPerRow, size_t bytesPerImage)
{
	myDeviceContext->UpdateSubresource(resource, 0, nullptr, data, static_cast<UINT>(bytesPerRow), static_cast<UINT>(bytesPerImage));
}

void
DX11Device::generateMips(ID3D11ShaderResourceView *view)
{
	myDeviceContext->GenerateMips(view);
}

void
DX11Device::setConstantBuffer(ID3D11Buffer * buffer)
{
	myDeviceContext->VSSetConstantBuffers(0, 1, &buffer);
}

void
DX11Device::drawIndexed(int count)
{
	myDeviceContext->DrawIndexed(count, 0, 0);
}

void
DX11Device::stop()
{
	myDeviceContext->ClearState();
	releaseEverything();
}

std::wstring
DX11Device::getResourcePath() const
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

HRESULT
DX11Device::configureBackBuffer(bool depth)
{
	HRESULT result = mySwapChain->GetBuffer(0, IID_PPV_ARGS(&myBackBuffer));

	if (SUCCEEDED(result))
	{
		result = myDevice->CreateRenderTargetView(myBackBuffer.Get(), nullptr, &myRenderTarget);
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

			myDevice->CreateDepthStencilView(myDepthStencil.Get(), &depthStencilViewDesc, &myDepthStencilView);
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

void
DX11Device::releaseEverything()
{
	releaseBackBuffer();
	mySwapChain.Reset();
	myDeviceContext.Reset();
	myDevice.Reset();
}

HRESULT
DX11Device::releaseBackBuffer()
{
	myRenderTarget.Reset();
	myBackBuffer.Reset();
	myDepthStencilView.Reset();
	myDepthStencil.Reset();
	if (myDeviceContext)
	{
		myDeviceContext->Flush();
	}
	return S_OK;
}

ComPtr<ID3D11Buffer>
DX11Device::loadBuffer(unsigned int size, D3D11_BIND_FLAG flags, const void *data)
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

	ComPtr<ID3D11Buffer> buffer;

	HRESULT result = myDevice->CreateBuffer(&bufferDescription, data ? &vertexBufferData : nullptr, &buffer);

	if (SUCCEEDED(result))
	{
		return buffer;
	}
	return nullptr;
}

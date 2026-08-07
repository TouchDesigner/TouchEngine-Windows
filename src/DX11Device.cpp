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
#include "Strings.h"

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

	myDeviceName = ConvertToMultiByte(description);

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
DX11Device::createWindowResources(HWND window)
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
		result = configureBackBuffer();
	}

	return result;
}

HRESULT
DX11Device::resize()
{
	myDeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
	
	releaseBackBuffer();

	HRESULT result = mySwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
	if (SUCCEEDED(result))
	{
		result = configureBackBuffer();
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

Microsoft::WRL::ComPtr<ID3D11BlendState> DX11Device::createBlendState()
{
	CD3D11_BLEND_DESC description(D3D11_DEFAULT);
	
	description.RenderTarget[0].BlendEnable = true;
	description.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	description.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	description.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	description.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	description.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	description.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	description.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	Microsoft::WRL::ComPtr<ID3D11BlendState> state;

	HRESULT result = myDevice->CreateBlendState(&description, &state);
	if (SUCCEEDED(result))
	{
		return state;
	}
	return Microsoft::WRL::ComPtr<ID3D11BlendState>();
}

DX11Texture
DX11Device::loadTexture(const unsigned char * src, int bytesPerRow, int width, int height)
{
	return DX11Texture(*this, src, bytesPerRow, width, height, true);
}

void
DX11Device::setRenderTarget()
{
	myDeviceContext->OMSetRenderTargets(1, myRenderTarget.GetAddressOf(), nullptr);
}

void
DX11Device::clear(const Color &color)
{
	const float c[4] = { color.red, color.green, color.blue, color.alpha };
	myDeviceContext->ClearRenderTargetView(myRenderTarget.Get(), c);
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

void DX11Device::setBlendState(ID3D11BlendState* state)
{
	myDeviceContext->OMSetBlendState(state, nullptr, 0xffffffff);
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
DX11Device::configureBackBuffer()
{
	HRESULT result = mySwapChain->GetBuffer(0, IID_PPV_ARGS(&myBackBuffer));

	if (SUCCEEDED(result))
	{
		result = myDevice->CreateRenderTargetView(myBackBuffer.Get(), nullptr, &myRenderTarget);
	}
	if (SUCCEEDED(result))
	{
		D3D11_TEXTURE2D_DESC	backBufferDescription{ };

		myBackBuffer->GetDesc(&backBufferDescription);

		myViewport = CD3D11_VIEWPORT(
			0.0f,
			0.0f,
			static_cast<float>(backBufferDescription.Width),
			static_cast<float>(backBufferDescription.Height));

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
	if (myDeviceContext)
	{
		myDeviceContext->Flush();
	}
	return S_OK;
}

ComPtr<ID3D11Buffer>
DX11Device::loadBuffer(unsigned int size, D3D11_BIND_FLAG flags, const void *data)
{
	CD3D11_BUFFER_DESC bufferDescription(
		size,
		flags,
		D3D11_USAGE_DEFAULT,
		0,
		0,
		0
	);

	D3D11_SUBRESOURCE_DATA vertexBufferData = {
		.pSysMem = data,
		.SysMemPitch = 0,
		.SysMemSlicePitch = 0
	};

	ComPtr<ID3D11Buffer> buffer;

	HRESULT result = myDevice->CreateBuffer(&bufferDescription, data ? &vertexBufferData : nullptr, &buffer);

	if (SUCCEEDED(result))
	{
		return buffer;
	}
	return nullptr;
}

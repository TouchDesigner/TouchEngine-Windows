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

#pragma once

#include <string>
#include "DX11VertexShader.h"
#include "DX11Texture.h"

class DX11Device
{
public:
	DX11Device();
	DX11Device(const DX11Device &o) = delete;
	DX11Device &operator=(const DX11Device &o) = delete;

	HRESULT		createDeviceResources();
	HRESULT		createWindowResources(HWND window, bool depth);
	HRESULT		resize();

	DX11VertexShader							loadVertexShader(const std::wstring &file, const D3D11_INPUT_ELEMENT_DESC *layout, int count);
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	loadPixelShader(const std::wstring &file);
	Microsoft::WRL::ComPtr<ID3D11Buffer>		loadIndexBuffer(unsigned short *indices, int count);

	template <class T>
	Microsoft::WRL::ComPtr<ID3D11Buffer>
	loadVertexBuffer(T* vertices, int count)
	{
		return loadBuffer(sizeof(T) * count, D3D11_BIND_VERTEX_BUFFER, vertices);
	}

	template <class T>
	Microsoft::WRL::ComPtr<ID3D11Buffer>
	loadConstantBuffer()
	{
		return loadBuffer(sizeof(T), D3D11_BIND_CONSTANT_BUFFER, nullptr);
	}

	DX11Texture	loadTexture(const unsigned char *src, int bytesPerRow, int width, int height);
	void			setRenderTarget();
	void			clear(float r, float g, float b, float a);
	void			present();
	void			setInputLayout(DX11VertexShader &shader);

	template <class T>
	void
	setVertexBuffer(ID3D11Buffer* buffer)
	{
		UINT stride = sizeof(T);
		UINT offset = 0;
		myDeviceContext->IASetVertexBuffers(0, 1, &buffer, &stride, &offset);
	}

	void	setIndexBuffer(ID3D11Buffer *buffer);
	void	setTriangleStripTopology();
	void	setVertexShader(DX11VertexShader &shader);
	void	setPixelShader(ID3D11PixelShader *shader);
	void	setShaderResourceAndSampler(DX11Texture &texture);
	void	updateSubresource(ID3D11Resource *resource, const void *data);
	void	updateSubresource(ID3D11Resource *resource, const void *data, size_t bytesPerRow, size_t bytesPerImage);
	void	generateMips(ID3D11ShaderResourceView *view);
	void	setConstantBuffer(ID3D11Buffer *buffer);
	void	drawIndexed(int count);
	void	stop();

	ID3D11Device*
	getDevice() const
	{
		return myDevice.Get();
	}

	const std::wstring& getDeviceName() const
	{
		return myDeviceName;
	}
private:
	std::wstring							getResourcePath() const;
	HRESULT									configureBackBuffer(bool depth);
	void									releaseEverything();
	HRESULT									releaseBackBuffer();
	Microsoft::WRL::ComPtr<ID3D11Buffer>	loadBuffer(unsigned int size, D3D11_BIND_FLAG flags, const void *data);

	Microsoft::WRL::ComPtr<ID3D11Device>			myDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>		myDeviceContext;
	D3D_FEATURE_LEVEL								myFeatureLevel{ D3D_FEATURE_LEVEL_1_0_CORE };
	Microsoft::WRL::ComPtr<IDXGISwapChain>			mySwapChain;
	Microsoft::WRL::ComPtr<ID3D11Texture2D>			myBackBuffer;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView>	myRenderTarget;
	Microsoft::WRL::ComPtr<ID3D11Texture2D>			myDepthStencil;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView>	myDepthStencilView;

	D3D11_TEXTURE2D_DESC	myBackBufferDescription{ };
	D3D11_VIEWPORT			myViewport{};

	std::wstring				myDeviceName;
};


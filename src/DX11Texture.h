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

#include <cstdint>
#include <TouchEngine/TouchObject.h>

class DX11Device;

class DX11Texture
{
public:
	DX11Texture();
	DX11Texture(DX11Device &device, const unsigned char *src, int bytesPerRow, int width, int height, bool genMips = false);
	DX11Texture(const TouchObject<TED3D11Texture> &texture);

	ID3D11Texture2D*	getTexture() const;
	bool				isValid() const;
	void				setResourceAndSampler(ID3D11DeviceContext* context);
	int					getWidth() const;
	int					getHeight() const;
	bool				getFlipped() const;
	void				acquire(uint64_t value);
	void				release(uint64_t value);
	constexpr uint64_t
		getLastAcquireValue() const
	{
		return myLastAcquireValue;
	}
private:
	HRESULT				createShaderResourceView(ID3D11Device *device, const D3D11_TEXTURE2D_DESC &description);
	HRESULT				createSamplerState(ID3D11Device *device, const D3D11_TEXTURE2D_DESC &description);
	void				releaseResources();


	TouchObject<TED3D11Texture>							mySource;
	Microsoft::WRL::ComPtr<ID3D11Texture2D>				myTexture;
	Microsoft::WRL::ComPtr<IDXGIKeyedMutex>				myKeyedMutex;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	myTextureView;
	Microsoft::WRL::ComPtr<ID3D11SamplerState>			mySampler;
	bool												myVFlipped{ false };
	uint64_t											myLastAcquireValue{ 0 };
};


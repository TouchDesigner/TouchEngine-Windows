/* Shared Use License: This file is owned by Derivative Inc. (Derivative) and
 * can only be used, and/or modified for use, in conjunction with
 * Derivative's TouchDesigner software, and only if you are a licensee who has
 * accepted Derivative's TouchDesigner license or assignment agreement (which
 * also govern the use of this file).  You may share a modified version of this
 * file with another authorized licensee of Derivative's TouchDesigner software.
 * Otherwise, no redistribution or sharing of this file, with or without
 * modification, is permitted.
 *
 * TouchEngine
 *
 * Copyright © 2021 Derivative. All rights reserved.
 *
 */

#pragma once
class DirectXDevice;

class DirectXTexture
{
public:
	DirectXTexture();
	DirectXTexture(DirectXDevice &device, const unsigned char *src, int bytesPerRow, int width, int height, bool genMips = false);
	DirectXTexture(ID3D11Texture2D *texture, bool flipped);
	DirectXTexture(const DirectXTexture &o);
	DirectXTexture &operator=(const DirectXTexture &o);
	DirectXTexture(DirectXTexture &&o);
	DirectXTexture &operator=(DirectXTexture &&o);
	~DirectXTexture();
	ID3D11Texture2D *getTexture() const;
	bool isValid() const;
	void setResourceAndSampler(ID3D11DeviceContext *context);
	int getWidth() const;
	int getHeight() const;
	bool getFlipped() const;
private:
	HRESULT createShaderResourceView(ID3D11Device *device, const D3D11_TEXTURE2D_DESC &description);
	HRESULT createSamplerState(ID3D11Device *device, const D3D11_TEXTURE2D_DESC &description);
	void releaseResources();
	ID3D11Texture2D *myTexture;
	ID3D11ShaderResourceView *myTextureView;
	ID3D11SamplerState *mySampler;
	bool myVFlipped;
};


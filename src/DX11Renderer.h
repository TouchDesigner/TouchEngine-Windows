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
#include "Renderer.h"
#include "DX11VertexShader.h"
#include "DX11Image.h"
#include "DX11Device.h"
#include <vector>

class DX11Renderer :
	public Renderer
{
public:
	DX11Renderer();
	virtual ~DX11Renderer();

	virtual Graphics getMode() const override
	{
		return Graphics::DX11;
	}

	virtual TouchObject<TEGraphicsContext>
	getTEContext() const override
	{
		return myContext;
	}

	virtual void	setup(HWND window) override;
	virtual bool	configure(TEInstance* instance, std::string& error) override;
	virtual void	resize(int width, int height) override;
	virtual void	stop() override;
	virtual bool	render() override;


	virtual TouchObject<TETexture>	getTexture(const unsigned char* rgba, size_t bytesPerRow, int width, int height) override;
	virtual void		clearInputs() override;
	virtual bool		setOutputImage(const TouchObject<TETexture>& texture, const TouchObject<TESemaphore>& semaphore, uint64_t waitValue) override;
	virtual void		setOutputImage(const TouchObject<TETexture>& texture) override;
	virtual void		clearOutputs() override;

	ID3D11Device*
	getDevice() const
	{
		return myDevice.getDevice();
	}

	virtual const std::string& getDeviceName() const override;
private:
	void		drawImage(DX11Image &image, float scale, float xOffset, float yOffset);

	DX11Device									myDevice;
	TouchObject<TED3D11Context>					myContext;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	myPixelShader;
	Microsoft::WRL::ComPtr<ID3D11BlendState>	myBlendState;
	DX11VertexShader							myVertexShader;
	DX11Texture									myInputTexture;
	DX11Image									myOutputImage;
	bool										myReleaseToZero{ false };
};


#include "stdafx.h"
#include "DirectXRenderer.h"
#include "DirectXDevice.h"
#include <TouchEngine/TouchEngine.h>
#include <array>

DirectXRenderer::DirectXRenderer()
    : Renderer(), myDevice()
{
}


DirectXRenderer::~DirectXRenderer()
{
    
}

bool DirectXRenderer::setup(HWND window)
{
	Renderer::setup(window);
	HRESULT result = myDevice.createDeviceResources();
	if (SUCCEEDED(result))
	{
		// Create window resources with no depth-stencil buffer
		result = myDevice.createWindowResources(getWindow(), false);
	}
    if (SUCCEEDED(result))
    {
        myPixelShader = myDevice.loadPixelShader(L"TestPixelShader.cso");
        if (!myPixelShader)
        {
            result = EIO;
        }
    }
    if (SUCCEEDED(result))
    {
        const D3D11_INPUT_ELEMENT_DESC layoutDescription[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };

        myVertexShader = myDevice.loadVertexShader(L"TestVertexShader.cso", layoutDescription, ARRAYSIZE(layoutDescription));
        if (!myVertexShader.isValid())
        {
            result = EIO;
        }
    }
	if (SUCCEEDED(result))
	{
		if (TED3D11ContextCreate(myDevice.getDevice(), &myContext) != TEResultSuccess)
		{
			result = E_FAIL;
		}
	}
    return SUCCEEDED(result);
}

void DirectXRenderer::resize(int width, int height)
{
    Renderer::resize(width, height);
	myDevice.resize();
}

void DirectXRenderer::stop()
{
    myLeftSideImages.clear();
    myRightSideImages.clear();
    if (myPixelShader)
    {
        myPixelShader->Release();
        myPixelShader = nullptr;
    }
    // Invalidate the vertex shader
    myVertexShader = VertexShader();
	myDevice.stop();
}

bool DirectXRenderer::render()
{ 
    myDevice.setRenderTarget();
    myDevice.clear(myBackgroundColor[0], myBackgroundColor[1], myBackgroundColor[2], 1.0f);

    myDevice.setPixelShader(myPixelShader);
    myDevice.setInputLayout(myVertexShader);
    myDevice.setVertexShader(myVertexShader);

    float scale = 1.0f / (max(myLeftSideImages.size(), myRightSideImages.size()) + 1.0f);
    drawImages(myLeftSideImages, scale, -0.5f);
    drawImages(myRightSideImages, scale, 0.5f);

    myDevice.present();
    return true;
}

void DirectXRenderer::addLeftSideImage(const unsigned char * rgba, size_t bytesPerRow, int width, int height)
{
	DirectXTexture texture = myDevice.loadTexture(rgba, int32_t(bytesPerRow), width, height);

	myLeftSideImages.emplace_back(texture);
	myLeftSideImages.back().setup(myDevice);
}

TETexture * DirectXRenderer::createLeftSideImage(size_t index)
{
	auto &texture = myLeftSideImages[index];
	return TED3D11TextureCreate(texture.getTexture(), false);
}

void DirectXRenderer::clearLeftSideImages()
{
    myLeftSideImages.clear();
}

void DirectXRenderer::addRightSideImage()
{
    myRightSideImages.emplace_back();
    myRightSideImages.back().setup(myDevice);

	Renderer::addRightSideImage();
}

void DirectXRenderer::setRightSideImage(size_t index, TETexture * texture)
{
	bool success = false;
	if (texture && TETextureGetType(texture) == TETextureTypeDXGI)
	{
		TED3D11Texture *created = nullptr;
		if (TED3D11ContextCreateTexture(myContext, static_cast<TEDXGITexture *>(texture), &created) == TEResultSuccess)
		{
			DirectXTexture tex(TED3D11TextureGetTexture(created), TETextureIsVerticallyFlipped(created));

			myRightSideImages.at(index).update(tex);

			// Renderer will TERetain the texture for us
			Renderer::setRightSideImage(index, created);

			TERelease(&created);

			success = true;
		}
	}

	if (!success)
	{
		myRightSideImages.at(index).update(DirectXTexture());
		Renderer::setRightSideImage(index, nullptr);
	}
}

void DirectXRenderer::clearRightSideImages()
{
    myRightSideImages.clear();

	Renderer::clearRightSideImages();
}

void DirectXRenderer::drawImages(std::vector<DirectXImage>& images, float scale, float xOffset)
{
    float numImages = (1.0f / scale) - 1.0f;
    float spacing = 1.0f / numImages;
    float yOffset = 1.0f - spacing;
    float ratio = static_cast<float>(myHeight) / myWidth;
    for (auto &image : images)
    {
        image.scale(scale * ratio, scale);
        image.position(xOffset, yOffset);
        image.draw(myDevice);
        yOffset -= spacing * 2;
    }
}

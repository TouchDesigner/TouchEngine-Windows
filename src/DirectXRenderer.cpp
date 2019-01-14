#include "stdafx.h"
#include "DirectXRenderer.h"
#include "DirectXDevice.h"
#include <TouchEngine.h>
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
    return SUCCEEDED(result);
}

void DirectXRenderer::resize(int width, int height)
{
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
	return TED3DTextureCreate(texture.getTexture());
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
	TETexture *d3d = nullptr;
	if (texture && TETextureGetType(texture) == TETextureTypeD3D)
	{
		// Retain as we release d3d and texture
		d3d = TERetain(texture);
	}
	else if (texture && TETextureGetType(texture) == TETextureTypeDXGI)
	{
		d3d = TED3DTextureCreateFromDXGI(myDevice.getDevice(), texture);
	}
	else if (texture && TETextureGetType(texture) == TETextureTypeOpenGL)
	{
		// TODO: or document ouTEut is always TETextureTypeDXGI
	}
	if (d3d)
	{
		DirectXTexture tex(TED3DTextureGetTexture(d3d));

		myRightSideImages.at(index).update(tex);
	}
	else
	{
		myRightSideImages.at(index).update(DirectXTexture());
	}

	// TODO: work out lifetime of which what what?
	Renderer::setRightSideImage(index, d3d);
	//Renderer::setRightSideImage(index, texture);
	
	if (d3d)
	{
		TERelease(&d3d);
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
    for (auto &image : images)
    {
        image.scale(scale);
        image.position(xOffset, yOffset);
        image.draw(myDevice);
        yOffset -= spacing * 2;
    }
}

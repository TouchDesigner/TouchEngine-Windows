#include "stdafx.h"
#include "DirectXRenderer.h"
#include "DirectXDevice.h"
#include <array>

DirectXRenderer::DirectXRenderer(DirectXDevice &device)
    : myDevice(device)
{
}


DirectXRenderer::~DirectXRenderer()
{
    
}

bool DirectXRenderer::setup()
{
    return true;
}

void DirectXRenderer::stop()
{
    myLeftSideImages.clear();
    myRightSideImages.clear(); 
}

bool DirectXRenderer::render()
{
    std::lock_guard<std::mutex> guard(myMutex);
 
    myDevice.setRenderTarget();
    myDevice.clear(myBackgroundColor[0], myBackgroundColor[1], myBackgroundColor[2], 1.0f);

    float scale = 1.0 / (max(myLeftSideImages.size(), myRightSideImages.size()) + 1);
    drawImages(myLeftSideImages, scale, -0.5f);
    drawImages(myRightSideImages, scale, 0.5f);

    myDevice.present();
    return true;
}

void DirectXRenderer::addLeftSideTexture(DirectXTexture & texture)
{
    std::lock_guard<std::mutex> guard(myMutex);

    myLeftSideImages.emplace_back(texture);
    myLeftSideImages.back().setup(myDevice);
}

void DirectXRenderer::clearLeftSideImages()
{
    std::lock_guard<std::mutex> guard(myMutex);

    myLeftSideImages.clear();
}

void DirectXRenderer::addRightSideImage()
{
    std::lock_guard<std::mutex> guard(myMutex);

    myRightSideImages.emplace_back();
    myRightSideImages.back().setup(myDevice);
}

void DirectXRenderer::replaceRightSideTexture(size_t index, DirectXTexture & texture)
{
    std::lock_guard<std::mutex> guard(myMutex);

    myRightSideImages.at(index).update(texture);
}

void DirectXRenderer::clearRightSideImages()
{
    std::lock_guard<std::mutex> guard(myMutex);

    myRightSideImages.clear();
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

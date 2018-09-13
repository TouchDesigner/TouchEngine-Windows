#pragma once
#include "Renderer.h"
#include "VertexShader.h"
#include "DirectXImage.h"
#include <vector>
#include <mutex>

class DirectXDevice;

class DirectXRenderer :
    public Renderer
{
public:
    DirectXRenderer(DirectXDevice &device);
    virtual ~DirectXRenderer();
    virtual bool setup() override;
    void stop();
    virtual bool render() override;
    size_t getLeftSideImageCount() const { return myLeftSideImages.size(); }
    const DirectXImage &getLeftSideImage(int index) const { return myLeftSideImages.at(index); }
    void addLeftSideTexture(DirectXTexture &texture);
    void clearLeftSideImages();
    size_t getRightSideImageCount() const { return myRightSideImages.size(); }
    const DirectXImage &getRightSideImage(int index) const { return myRightSideImages.at(index); }
    void addRightSideImage();
    void replaceRightSideTexture(size_t index, DirectXTexture &texture);
    void clearRightSideImages();
private:
    void drawImages(std::vector<DirectXImage> &images, float scale, float xOffset);
    DirectXDevice &myDevice;
    std::vector<DirectXImage> myLeftSideImages;
    std::vector<DirectXImage> myRightSideImages;
    std::mutex myMutex;
};


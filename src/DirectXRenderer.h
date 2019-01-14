#pragma once
#include "Renderer.h"
#include "VertexShader.h"
#include "DirectXImage.h"
#include "DirectXDevice.h"
#include <vector>
#include <mutex>

class DirectXRenderer :
    public Renderer
{
public:
    DirectXRenderer();
    virtual ~DirectXRenderer();
    virtual bool setup(HWND window) override;
	virtual void resize(int width, int height) override;
    virtual void stop() override;
    virtual bool render() override;
    virtual size_t getLeftSideImageCount() const override { return myLeftSideImages.size(); }
	virtual void addLeftSideImage(const unsigned char *rgba, size_t bytesPerRow, int width, int height);
	virtual TETexture *createLeftSideImage(size_t index) override;
    virtual void clearLeftSideImages() override;
    virtual void addRightSideImage() override;
	virtual void setRightSideImage(size_t index, TETexture *texture) override;
    virtual void clearRightSideImages() override;
private:
    void drawImages(std::vector<DirectXImage> &images, float scale, float xOffset);
    DirectXDevice myDevice;
    ID3D11PixelShader *myPixelShader;
    VertexShader myVertexShader;
    std::vector<DirectXImage> myLeftSideImages;
    std::vector<DirectXImage> myRightSideImages;
};


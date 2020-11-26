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
	void fillAndGenerateMips(ID3D11DeviceContext *context, const unsigned char *src, int bytesPerRow, int width, int height);
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


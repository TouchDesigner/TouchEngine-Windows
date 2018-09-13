#pragma once
class DirectXTexture
{
public:
    DirectXTexture();
    DirectXTexture(ID3D11Device *device, const unsigned char *src, int bytesPerRow, int width, int height);
    DirectXTexture(ID3D11Texture2D *texture);
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
private:
    HRESULT createShaderResourceView(ID3D11Device *device, const D3D11_TEXTURE2D_DESC &description);
    HRESULT createSamplerState(ID3D11Device *device, const D3D11_TEXTURE2D_DESC &description);
    void releaseResources();
    ID3D11Texture2D *myTexture;
    ID3D11ShaderResourceView *myTextureView;
    ID3D11SamplerState *mySampler;
};


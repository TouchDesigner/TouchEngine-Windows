#pragma once
class DirectXTexture
{
public:
    DirectXTexture();
    DirectXTexture(ID3D11Device *device, const unsigned char *src, int bytesPerRow, int width, int height);
    DirectXTexture(const DirectXTexture &o) = delete;
    DirectXTexture &operator=(const DirectXTexture &o) = delete;
    DirectXTexture(DirectXTexture &&o);
    DirectXTexture &operator=(DirectXTexture &&o);
    ~DirectXTexture();
    ID3D11Texture2D *getTexture() const;
    bool isValid() const;
    void setResourceAndSampler(ID3D11DeviceContext *context);
private:
    void releaseResources();
    ID3D11Texture2D *myTexture;
    ID3D11ShaderResourceView *myTextureView;
    ID3D11SamplerState *mySampler;
    int myWidth;
    int myHeight;
};


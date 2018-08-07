#pragma once



class VertexShader
{
public:
    VertexShader();
    VertexShader(ID3D11VertexShader *shader, ID3D11InputLayout *layout);
    VertexShader(const VertexShader &o) = delete;
    VertexShader &operator=(const VertexShader &o) = delete;
    VertexShader(VertexShader &&o);
    ~VertexShader();
    VertexShader &operator=(VertexShader &&o);
    bool isValid() const;
    void setInputLayout(ID3D11DeviceContext *context);
    void setShader(ID3D11DeviceContext *context);
private:
    ID3D11VertexShader *myVertexShader;
    ID3D11InputLayout *myInputLayout;
};


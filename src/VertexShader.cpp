#include "stdafx.h"
#include "VertexShader.h"


VertexShader::VertexShader()
    : myVertexShader(nullptr), myInputLayout(nullptr)
{
}

VertexShader::VertexShader(ID3D11VertexShader * shader, ID3D11InputLayout * layout)
    :myVertexShader(shader), myInputLayout(layout)
{
    myVertexShader->AddRef();
    myInputLayout->AddRef();
}


VertexShader::VertexShader(VertexShader && o)
    : myVertexShader(o.myVertexShader), myInputLayout(o.myInputLayout)
{
    o.myVertexShader = nullptr;
    o.myInputLayout = nullptr;
}

VertexShader & VertexShader::operator=(VertexShader && o)
{
    if (myVertexShader)
    {
        myVertexShader->Release();
    }
    if (myInputLayout)
    {
        myInputLayout->Release();
    }
    myVertexShader = o.myVertexShader;
    myInputLayout = o.myInputLayout;
    o.myVertexShader = nullptr;
    o.myInputLayout = nullptr;
    return *this;
}

bool VertexShader::isValid() const
{
    if (myVertexShader && myInputLayout)
    {
        return true;
    }
    return false;
}

void VertexShader::setInputLayout(ID3D11DeviceContext * context)
{
    context->IASetInputLayout(myInputLayout);
}

void VertexShader::setShader(ID3D11DeviceContext * context)
{
    context->VSSetShader(myVertexShader, nullptr, 0);
}

VertexShader::~VertexShader()
{
    if (myVertexShader)
    {
        myVertexShader->Release();
    }
    if (myInputLayout)
    {
        myInputLayout->Release();
    }
}

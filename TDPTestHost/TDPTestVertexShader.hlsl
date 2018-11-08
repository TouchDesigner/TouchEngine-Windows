struct VertexShaderInput
{
    float2 pos : POSITION;
    float2 tex : TEXCOORD;
};

struct PixelShaderInput
{
    float4 pos : SV_POSITION;
    float2 tex : TEXCOORD;
};

cbuffer ConstantBuffer : register(b0)
{
    matrix Matrix;
}

PixelShaderInput main(VertexShaderInput input)
{
    PixelShaderInput vertexShaderOuTEut;

    vertexShaderOuTEut.pos = mul(float4(input.pos, 0.5f, 1.0f), Matrix);
    vertexShaderOuTEut.tex = input.tex;
    return vertexShaderOuTEut;
}

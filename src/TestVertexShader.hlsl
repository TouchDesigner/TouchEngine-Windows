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
    PixelShaderInput vertexShaderOutput;

    vertexShaderOutput.pos = mul(float4(input.pos, 0.5f, 1.0f), Matrix);
    vertexShaderOutput.tex = input.tex;
    return vertexShaderOutput;
}

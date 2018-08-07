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

PixelShaderInput main(VertexShaderInput input)
{
    PixelShaderInput vertexShaderOutput;

    vertexShaderOutput.pos = float4(input.pos, 0.5f, 1.0f);
    vertexShaderOutput.tex = input.tex;
    return vertexShaderOutput;
}

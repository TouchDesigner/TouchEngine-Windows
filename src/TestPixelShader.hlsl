Texture2D tex : register(t0);
SamplerState samp : register(s0);

struct PixelShaderInput
{
    float4 pos : SV_POSITION;
    float2 tex: TEXCOORD;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
    return tex.Sample(samp, input.tex);
}

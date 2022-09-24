/* Shared Use License: This file is owned by Derivative Inc. (Derivative)
* and can only be used, and/or modified for use, in conjunction with
* Derivative's TouchDesigner software, and only if you are a licensee who has
* accepted Derivative's TouchDesigner license or assignment agreement
* (which also govern the use of this file). You may share or redistribute
* a modified version of this file provided the following conditions are met:
*
* 1. The shared file or redistribution must retain the information set out
* above and this list of conditions.
* 2. Derivative's name (Derivative Inc.) or its trademarks may not be used
* to endorse or promote products derived from this file without specific
* prior written permission from Derivative.
*/

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
    int4 Flip;
}

PixelShaderInput main(VertexShaderInput input)
{
    PixelShaderInput vertexShaderOutput;

    vertexShaderOutput.pos = mul(float4(input.pos, 0.5f, 1.0f), Matrix);
    vertexShaderOutput.tex = input.tex;
    if (Flip.x)
    {
        vertexShaderOutput.tex.y = 1.0f - vertexShaderOutput.tex.y;
    }
    return vertexShaderOutput;
}

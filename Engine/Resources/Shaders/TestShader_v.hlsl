#include "TestShader_VSOut.hlsl"

cbuffer MVPConstants : register(b0)
{
    matrix MVP;
    matrix Model;
};

VSOut
main(float3 pos : Position, float3 norm : Normal, float2 uv : TexCoord)
{
    VSOut vso;
    vso.svpos = mul(MVP, float4(pos, 1.0f));
    vso.norm = mul((float3x3) Model, norm);
    vso.uv = uv;
    return vso;
}
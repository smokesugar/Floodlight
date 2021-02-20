#include "TestShader_VSOut.hlsl"

cbuffer MVPConstants : register(b0)
{
    matrix MVP;
};

VSOut
main(float3 pos : Position, float2 uv : TexCoord)
{
    VSOut vso;
    vso.svpos = mul(MVP, float4(pos, 1.0f));
    vso.uv = uv;
    return vso;
}
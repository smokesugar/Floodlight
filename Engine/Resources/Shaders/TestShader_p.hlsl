#include "TestShader_VSOut.hlsl"

SamplerState LinearSampler : register(s0);
Texture2D MyTexture : register(t0);

float4
main(VSOut vso) : SV_Target
{
    float3 sampledColor = MyTexture.Sample(LinearSampler, vso.uv);
    float gamma = 1.0f;
    return float4(pow(sampledColor, 1 / gamma), 1.0f);
}
#include "TestShader_VSOut.hlsl"

SamplerState LinearSampler : register(s0);
Texture2D MyTexture : register(t0);

float4
main(VSOut vso) : SV_Target
{
    float3 sampledColor = MyTexture.Sample(LinearSampler, vso.uv);
    
    float3 N = normalize(vso.norm);
    float3 L = normalize(float3(-1.0f, 2.0f, -1.0f));
    
    float lighting = max(dot(N, L), 0.0f);
    float3 fragColor = sampledColor * lighting;
    
    float gamma = 1.0f;
    return float4(pow(fragColor, 1 / gamma), 1.0f);
}
#include "TestShader_VSOut.hlsl"

float4
main(VSOut vso) : SV_Target
{
    float gamma = 2.2f;
    return float4(pow(vso.col.rgb, 1 / gamma), 1.0f);
}
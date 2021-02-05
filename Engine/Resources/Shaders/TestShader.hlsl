
struct VSOut
{
    float4 svpos : SV_Position;
    float4 col : Color;
};

cbuffer MVPConstants : register(b0)
{
    matrix MVP;
};

cbuffer OffsetConstants : register(b1)
{
    matrix OffsetMatrix;
};

VSOut
VSMain(float3 pos : Position, float4 col : Color)
{
    VSOut vso;
    vso.svpos = mul(OffsetMatrix, mul(MVP, float4(pos, 1.0f)));
    vso.col = col;
    return vso;
}

float4
PSMain(VSOut vso) : SV_Target
{
    float gamma = 2.2f;
    return float4(pow(vso.col.rgb, 1/gamma), 1.0f);
}
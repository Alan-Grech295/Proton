cbuffer CBuf
{
    matrix viewProj;
};

struct VSOut
{
    float3 color : Color;
    float4 pos : SV_Position;
};

VSOut main(float3 pos : Position, float3 col : Color)
{
    VSOut vso;
    vso.pos = mul(float4(pos, 1.0f), viewProj);
    vso.color = col;
    return vso;
}
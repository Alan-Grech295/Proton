cbuffer CBuf
{
    matrix modelView;
    matrix modelViewProj;
};

struct VSOut
{
    float3 viewPos : POSITION;
    float3 normal : NORMAL;
    float2 tc : TEXCOORD;
    float4 pos : SV_POSITION;
};

VSOut main(float3 pos : POSITION, float3 n : NORMAL, float2 tc : TEXCOORD)
{
    VSOut vso;
    vso.viewPos = mul(float4(pos, 1.0f), modelView).xyz;
    vso.normal = mul(n, (float3x3) modelView).xyz;
    vso.pos = mul(float4(pos, 1.0f), modelViewProj);
    vso.tc = tc;
    return vso;
}
struct VSOut
{
    float2 uv : TEXCOORD;
    float4 pos : SV_POSITION;
};

VSOut main(uint id : SV_VERTEXID)
{
    VSOut vso;
    float2 uv = float2((id << 1) & 2, id & 2);
    vso.uv = uv;
    vso.pos = float4(uv * float2(2, -2) + float2(-1, 1), 0, 1);
    return vso;
}
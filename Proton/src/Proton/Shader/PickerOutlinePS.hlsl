cbuffer OutlineCBuf : register(b1)
{
    uint Thickness;
    float3 Color;
};

Texture2D<int> mask : register(t0);

bool IsOutline(float2 coord)
{
    if (mask[coord] > 0)
        return false;
    
    for (int y = coord.y - Thickness; y < coord.y + Thickness; y++)
    {
        for (int x = coord.x - Thickness; x < coord.x + Thickness; x++)
        {
            if (mask[int2(x, y)] > 0)
                return true;
        }
    }
    
    return false;
}

float4 main(float2 uv : TEXCOORD) : SV_TARGET
{
    uint width, height;
    mask.GetDimensions(width, height);
    if (IsOutline(uv * float2(width, height)))
    {
        return float4(Color, 1);
    }
    
    clip(-1);
    return 0;
}
float4 main(float3 colour : COLOUR) : SV_TARGET
{
	return float4(colour.xyz, 1.0f);
}
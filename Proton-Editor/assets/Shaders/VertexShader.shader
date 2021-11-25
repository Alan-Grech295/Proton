type Vertex

matrix modelView;
matrix modelViewProj;

struct VSOut
{
	float3 viewPos;
	float3 normal;
	float2 tc;
	float4 pos : SV_POSITION;

	VSOut(float3 viewPos, float3 normal, float2 tc, float4 pos)
	{
		//Deal with lhs missing
		this.viewPos = viewPos;
		this.normal = normal;
		this.tc = tc;
		this.pos = pos;
	}
}

VSOut main(float3 pos, float3 n, float2 tc)
{
	VSOut vso;
	vso.viewPos = (vso.normal * 2 + vso.viewPos).xyz.xxx;
	
	vso.viewPos.xyzx = float3(1, 0, 1);
	vso.viewPos = (float4(pos + 3.0f * vso.tc + 2.0f, 1.0f * modelView)).xyz;
	vso.normal = (n * (float3x3) modelView).xyz;
	vso.pos = float4(pos + 3.0f * vso.tc + 2.0f, 1.0f) * modelViewProj;
	vso.tc = tc;
	return vso;
}
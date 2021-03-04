cbuffer LightCBuf
{
    float3 lightPos;
    float3 ambient;
    float3 diffuseColor;
    float diffuseIntensity;
    float attConst;
    float attLin;
    float attQuad;
};

cbuffer ObjectCBuf
{
    float specularIntensity;
    float specularPower;
    float padding[2];
};

Texture2D tex;
SamplerState smplr;

float4 main(float3 viewPos : POSITION, float3 n : NORMAL, float2 tc : TEXCOORD) : SV_TARGET
{
	//fragment to light vector data
    const float3 vToL = lightPos - viewPos;
	const float distToL = length(vToL);
	const float3 dirToL = vToL / distToL;
	
	//diffuse attenuation
	float att = 1.0f / (attConst + attLin * distToL + attQuad * (distToL * distToL));
	
	//diffuse intensity
	float3 diffuse = diffuseColor * diffuseIntensity * att * max(0.0f, dot(dirToL, n));
	
	// reflected light vector
    const float3 w = n * dot(vToL, n);
    const float3 r = w * 2.0f - vToL;
	// calculate specular intensity based on angle between viewing vector and reflection vector, narrow with power function
    const float3 specular = att * (diffuseColor * diffuseIntensity) * specularIntensity * pow(max(0.0f, dot(normalize(-r), normalize(viewPos))), specularPower);
	
    return float4(saturate((diffuse + ambient) * tex.Sample(smplr, tc).rgb + specular), 1.0f);

}
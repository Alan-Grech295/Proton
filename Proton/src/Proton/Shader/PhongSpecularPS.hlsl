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

Texture2D tex;
Texture2D spec;
SamplerState smplr;

float4 main(float3 worldPos : POSITION, float3 n : NORMAL, float2 tc : TEXCOORD) : SV_TARGET
{
	//fragment to light vector data
    const float3 vToL = lightPos - worldPos;
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
    const float4 specularSample = spec.Sample(smplr, tc);
    const float3 specularColourIntensity = specularSample.rgb;
    const float specularPower = pow(2, specularSample.a * 13.0);
    
    const float3 specular = att * specularColourIntensity * pow(max(0.0f, dot(normalize(-r), normalize(worldPos))), specularPower);
    
    return float4(saturate((diffuse + ambient) * tex.Sample(smplr, tc).rgb + specular), 1.0f);
}
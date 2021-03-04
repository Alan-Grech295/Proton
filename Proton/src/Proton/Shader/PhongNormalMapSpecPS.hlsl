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
    bool hasAlphaGloss;
    float padding;
};

cbuffer CBuf
{
    matrix modelView;
    matrix modelViewProj;
};

Texture2D tex : register(t0);
Texture2D spec : register(t1);
Texture2D normalMap : register(t2);

SamplerState smplr;

float4 main(float3 viewPos : POSITION, float3 n : NORMAL, float3 tan : TANGENT, float3 bitan : BITANGENT, float2 tc : TEXCOORD) : SV_TARGET
{
    const float3x3 tanToView = float3x3(
        normalize(tan),
        normalize(bitan),
        normalize(n)
    );
    
    float4 normalSample = normalMap.Sample(smplr, tc);
    
    n = normalSample * 2.0f - 1.0f;
    n = mul(n, tanToView);
    
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
    
    
    
    const float4 specularSample = spec.Sample(smplr, tc);
    const float3 specularColourIntensity = specularSample.rgb;
    float specPower = specularPower;
    
    if(hasAlphaGloss)
    {
        specPower = pow(2, specularSample.a * 13.0);
    }
    
    
    const float3 specular = att * specularColourIntensity * pow(max(0.0f, dot(normalize(-r), normalize(viewPos))), specPower);
	
    return float4(saturate((diffuse + ambient) * tex.Sample(smplr, tc).rgb + specular), 1.0f);

}
#include "Includes/PointLight.hlsl"
#include "Includes/ShaderUtils.hlsl"
#include "Includes/LightVectorData.hlsl"

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

float4 main(float3 viewFragPos : POSITION, float3 viewNormal : NORMAL, float3 viewTan : TANGENT, float3 viewBitan : BITANGENT, float2 tc : TEXCOORD) : SV_TARGET
{
    //renormalize interpolated normal
    viewNormal = normalize(viewNormal);
    
    viewNormal = MapNormal(normalize(viewTan), normalize(viewBitan), viewNormal, tc, normalMap, smplr);
    
	// fragment to light vector data
    const LightVectorData lv = CalculateLightVectorData(viewLightPos, viewFragPos);
	
	//diffuse attenuation
    float att = Attenuation(attConst, attLin, attQuad, lv.distToL);
	
	//diffuse intensity
    float3 diffuse = Diffuse(diffuseColor, diffuseIntensity, att, lv.dirToL, viewNormal);

	// calculate specular intensity based on angle between viewing vector and reflection vector, narrow with power function
    const float4 specularSample = spec.Sample(smplr, tc);
    const float3 specularReflectionColour = specularSample.rgb;
    float specPower = specularPower;
    
    if(hasAlphaGloss)
    {
        specPower = pow(2, specularSample.a * 13.0);
    }
    
    const float3 specular = Specular(specularReflectionColour, specularIntensity, viewNormal, lv.vToL, viewFragPos, att, specPower);
	
    return float4(saturate((diffuse + ambient) * tex.Sample(smplr, tc).rgb + specular), 1.0f);

}
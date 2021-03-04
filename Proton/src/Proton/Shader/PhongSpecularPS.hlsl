#include "Includes/PointLight.hlsl"
#include "Includes/ShaderUtils.hlsl"
#include "Includes/LightVectorData.hlsl"

Texture2D tex;
Texture2D spec;
SamplerState smplr;

float4 main(float3 viewFragPos : POSITION, float3 viewNormal : NORMAL, float2 tc : TEXCOORD) : SV_TARGET
{
    //renormalize interpolated normal
    viewNormal = normalize(viewNormal);
        
	// fragment to light vector data
    const LightVectorData lv = CalculateLightVectorData(viewLightPos, viewFragPos);
	
	//diffuse attenuation
    float att = Attenuation(attConst, attLin, attQuad, lv.distToL);
	
	//diffuse intensity
    float3 diffuse = Diffuse(diffuseColor, diffuseIntensity, att, lv.dirToL, viewNormal);
	
	// calculate specular intensity based on angle between viewing vector and reflection vector, narrow with power function
    const float4 specularSample = spec.Sample(smplr, tc);
    const float3 specularColourIntensity = specularSample.rgb;
    const float specularPower = pow(2, specularSample.a * 13.0);
    
    const float3 specular = Specular(specularColourIntensity, 1.0f, viewNormal, lv.vToL, viewFragPos, att, specularPower);
        
    return float4(saturate((diffuse + ambient) * tex.Sample(smplr, tc).rgb + specular), 1.0f);
}
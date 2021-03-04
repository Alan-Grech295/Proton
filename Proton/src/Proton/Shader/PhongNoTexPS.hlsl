#include "Includes/PointLight.hlsl"
#include "Includes/ShaderUtils.hlsl"
#include "Includes/LightVectorData.hlsl"

cbuffer ObjectCBuf
{
    float4 materialColor;
    float4 specularColor;
    float specularPower;
};

float4 main(float3 viewFragPos : Position, float3 viewNormal : Normal) : SV_Target
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
    const float3 specular = Specular(diffuseColor, diffuseIntensity, viewNormal, lv.vToL, viewFragPos, att, specularPower);
    
	// final color
    return saturate(float4(diffuse + ambient, 1.0f) * materialColor + float4(specular, 1.0f));
}
#include "Includes/PointLightInc.hlsl"
#include "Includes/ShaderUtilsInc.hlsl"
#include "Includes/LightVectorDataInc.hlsl"

cbuffer ObjectCBuf
{
    float specularIntensity;
    float specularPower;
    bool hasAlphaGloss;
    int index = 50;
};

cbuffer CBuf
{
    matrix modelView;
    matrix modelViewProj;
};

struct PS_OUTPUT
{
    float4 Color : SV_Target0;
    int PickID : SV_Target1;
};

Texture2D tex : register(t0);
Texture2D spec : register(t1);
Texture2D normalMap : register(t2);

SamplerState smplr;

PS_OUTPUT main(float3 viewFragPos : POSITION, float3 viewNormal : NORMAL, float3 viewTan : TANGENT, float3 viewBitan : BITANGENT, float2 tc : TEXCOORD)
{
    PS_OUTPUT output;
    float4 texCol = tex.Sample(smplr, tc);
    //Discard if alpha too low
    clip(texCol.a < 0.1f ? -1 : 1);
    
    //Flip normal when backface
    if (dot(viewNormal, viewFragPos) >= 0.0f)
        viewNormal = -viewNormal;
    
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
	
    output.Color = float4(saturate((diffuse + ambient) * texCol.rgb + specular * diffuseIntensity), texCol.a);
    output.PickID = 50;
    
    return output;
}
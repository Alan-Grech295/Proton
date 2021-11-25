//Pixel Shader
/*
Nother multiline*/
type Pixel
#include "Includes/PointLight.shader"
#include "Includes/ShaderUtils.shader"
#include "Includes/LightVectorData.shader"

matrix modelView;
matrix modelViewProj;

float specularIntensity;
float specularPower;
bool hasAlphaGloss;
float padding;

Texture2D tex;
Texture2D spec;
Texture2D normalMap;

struct PS_OUTPUT
{
	float4 Color : SV_Target0;
	float4 PickID : SV_Target1;
};

PS_OUTPUT main(float3 viewFragPos, float3 viewNormal, float3 viewTan, float3 viewBitan, float2 tc)
{
	PS_OUTPUT output;

	/*
	This is a multiline
	comment
	*/

	if (tc.x > 0)
	{

	}

	if (tc.x < 0)
	{

	}
	if (tc.x >= 0)
	{

	}
	if (tc.x <= 0)
	{

	}

	if (tc.x == 0)
	{

	}

	if (tc.x != 0)
	{

	}


	//renormalize interpolated normal
	viewNormal = normalize(viewNormal);

	viewNormal = MapNormal(normalize(viewTan), normalize(viewBitan), viewNormal, tc, normalMap, smplr);

	// fragment to light vector data
	LightVectorData lv = CalculateLightVectorData(viewLightPos, viewFragPos);

	//diffuse attenuation
	float att = Attenuation(attConst, attLin, attQuad, lv.distToL);

	//diffuse intensity
	float3 diffuse = Diffuse(diffuseColor, diffuseIntensity, att, lv.dirToL, viewNormal);

	// calculate specular intensity based on angle between viewing vector and reflection vector, narrow with power function
	float4 specularSample = spec.Sample(smplr, tc);
	float3 specularReflectionColour = specularSample.rgb;
	float specPower = specularPower;

	if (hasAlphaGloss)
	{
		specPower = pow(2, specularSample.a * 13.0);
	}

	float3 specular = Specular(specularReflectionColour, specularIntensity, viewNormal, lv.vToL, viewFragPos, att, specPower);

	output.Color = float4(saturate((diffuse + ambient) * tex.Sample(smplr, tc).rgb + specular), 1.0f);
	output.PickID = 1;

	return output;
}
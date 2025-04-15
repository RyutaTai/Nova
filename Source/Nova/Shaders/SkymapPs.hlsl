#include "FullScreenQuad.hlsli"

cbuffer SCENE_CONSTANT_BUFFER : register(b5)
{
	row_major float4x4	viewProjection;
	float4				lightDirection;
	float4				cameraPosition;
	row_major float4x4	invViewProjection;
};

#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
SamplerState samplerStates[3] : register(s0);
Texture2D skybox : register(t5);

float4 SampleSkybox(float3 v, float roughness)
{
	const float PI = 3.14159265358979;

	uint width, height, numberOfLevels;
	skybox.GetDimensions(0, width, height, numberOfLevels);

	float lod = roughness * float(numberOfLevels - 1);
	v = normalize(v);

	// Blinn/Newell Latitude Mapping
	float2 samplePoint;
	samplePoint.x = (atan2(v.z, v.x) + PI) / (PI * 2.0);
	samplePoint.y = 1.0 - ((asin(v.y) + PI * 0.5) / PI);
	return skybox.SampleLevel(samplerStates[LINEAR], samplePoint, lod);
}

float4 main(VS_OUT pin) : SV_TARGET
{
	float4 ndc;
	ndc.x = (pin.texcoord.x * 2.0) - 1.0;
	ndc.y = 1.0 - (pin.texcoord.y * 2.0);
	ndc.z = 1;
	ndc.w = 1;

	float4 R = mul(ndc, invViewProjection);
	R /= R.w;
	const float roughness = 0;
	return SampleSkybox(R.xyz, roughness);
}

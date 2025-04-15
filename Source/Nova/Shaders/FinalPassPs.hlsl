#include "FullScreenQuad.hlsli"

#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
#define LINEAR_BORDER_BLACK 3
#define LINEAR_BORDER_WHITE 4
SamplerState samplerStates[5] : register(s0);
Texture2D textureMaps[2] : register(t0);

float3 ReinhardToneMapping(float3 color)
{
	float luma = dot(color, float3(0.2126, 0.7152, 0.0722));
	float toneMappedLuma = luma / (1. + luma);
	color *= toneMappedLuma / luma;
	return color;
}

float4 main(VS_OUT pin) : SV_TARGET
{
	float4 color = textureMaps[0].Sample(samplerStates[POINT], pin.texcoord);
	float4 bloom = textureMaps[1].Sample(samplerStates[POINT], pin.texcoord);

	float3 fragmentColor = color.rgb + bloom.rgb;
	float alpha = color.a;

	// Tone map
	fragmentColor = ReinhardToneMapping(fragmentColor);

	// Gamma correction
	const float INV_GAMMA = 1.0 / 2.2;
	fragmentColor = pow(fragmentColor, INV_GAMMA);

	return float4(fragmentColor, alpha);
}

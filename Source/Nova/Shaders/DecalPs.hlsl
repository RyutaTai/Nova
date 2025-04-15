#include "Decal.hlsli"

#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
#define LINEAR_BORDER_BLACK 3
#define LINEAR_BORDER_WHITE 4

SamplerState		samplerStates[5]	: register(s0);
Texture2D			decalTexture		: register(t0);
Texture2D<float>	sceneDepthTexture	: register(t1);

float4 main(float4 sv_position : SV_POSITION) : SV_TARGET
{
	float2 dimension;
	sceneDepthTexture.GetDimensions(dimension.x, dimension.y);
	float2 texcoord = sv_position.xy / dimension;
	float depth = sceneDepthTexture.Sample(samplerStates[LINEAR_BORDER_BLACK], texcoord);
	
	float4 ndc = float4(2.0 * texcoord.x - 1.0, 1.0 - 2.0 * texcoord.y, depth, 1.0);
	
	float4 position = mul(ndc, inverseViewProjection);
	position /= position.w;
	
	position = mul(position, decalInverseProjection);
	position /= position.w;

    float2 decalTexcoord = float2(position.x * +0.5f + 0.5f, position.y * -0.5f + 0.5f);

	float4 color = decalTexture.Sample(samplerStates[LINEAR_BORDER_BLACK], decalTexcoord);
	clip(color.a - 0.5);
	
	return float4(color.rgb, color.a);
}

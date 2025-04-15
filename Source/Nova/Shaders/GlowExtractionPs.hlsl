#include "Bloom.hlsli"

#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
#define LINEAR_BORDER_BLACK 3
#define LINEAR_BORDER_WHITE 4
SamplerState samplerStates[5] : register(s0);

Texture2D hdrColorBufferTexture : register(t0);
float4 main(float4 position : SV_POSITION, float2 texcoord : TEXCOORD) : SV_TARGET
{
	float4 sampledColor = hdrColorBufferTexture.Sample(samplerStates[POINT], texcoord);

#if 0
	return float4(step(bloomExtractionThreshold, dot(sampledColor.rgb, float3(0.299, 0.587, 0.114))) * sampledColor.rgb * effectData.bloomIntensity, sampledColor.a);
#else	
	return float4(step(bloomExtractionThreshold, max(sampledColor.r, max(sampledColor.g, sampledColor.b))) * sampledColor.rgb, sampledColor.a);
#endif	
}

#include "Bloom.hlsli"

#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
#define LINEAR_BORDER_BLACK 3
#define LINEAR_BORDER_WHITE 4
SamplerState samplerStates[5] : register(s0);

static const uint downsampledCount = 6;
Texture2D downsampledTextures[downsampledCount] : register(t0);

float4 main(float4 position : SV_POSITION, float2 texcoord : TEXCOORD) : SV_TARGET
{
	float3 sampledColor = 0;
	[unroll]
	for (uint downsampledIndex = 0; downsampledIndex < downsampledCount; ++downsampledIndex)
	{
		sampledColor += downsampledTextures[downsampledIndex].Sample(samplerStates[LINEAR], texcoord).xyz;
	}
	return float4(sampledColor * bloomIntensity, 1);
}

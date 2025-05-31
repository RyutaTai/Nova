#include "../../Nova/Shaders/Model.hlsli"

#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2

SamplerState samplerStates[3] : register(s0);
Texture2D textureMaps[4] : register(t0);

float4 main(VS_OUT pin) : SV_TARGET
{
    //float4 color = tex2D((sampler2D)samplerStates[ANISOTROPIC], pin.texcoord.xy);
    float4 color = textureMaps[0].Sample(samplerStates[ANISOTROPIC], pin.texcoord.xy);
    color.rgb = (color.r + color.g + color.b) * 0.3333f;
	
	return color;
}
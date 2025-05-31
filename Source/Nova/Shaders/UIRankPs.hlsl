#include "../../Nova/Shaders/Sprite.hlsli"

Texture2D colorMap : register(t0);

SamplerState pointSamplerState : register(s0);
cbuffer constant : register(b0)
{
    float threshold;
}

float4 main(VS_OUT pin) : SV_TARGET
{
    float4 color = colorMap.Sample(pointSamplerState, pin.texcoord);
    float alpha = color.a;
#if 1
    //Inverse gamma process(‹tƒKƒ“ƒ}•â³)
    const float GAMMA = 1 / 2.2;
    color.rgb = pow(color.rgb, GAMMA);
#endif
    //return color;
    float v = max(0, 1 - pin.texcoord.y);
    if (v > threshold)
        alpha = 0.0;
   
    return float4(color.rgb, alpha) * pin.color;
    
}
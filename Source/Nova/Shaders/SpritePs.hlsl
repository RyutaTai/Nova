#include "Sprite.hlsli"

Texture2D colorMap:register(t0);

SamplerState pointSamplerState : register(s0);

float4 main(VS_OUT pin) : SV_TARGET
{
    float4 color = colorMap.Sample(pointSamplerState, pin.texcoord);
    float alpha = color.a;
#if 1
    //Inverse gamma process(ãtÉKÉìÉ}ï‚ê≥)
    const float GAMMA = 1/2.2;
    color.rgb = pow(color.rgb, GAMMA);
#endif
    
    return float4(color.rgb, alpha) * pin.color;
    
}

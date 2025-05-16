#include "Sprite.hlsli"

cbuffer VignetteConstantBuffer : register(b2)
{
    float4 vignetteColor;
    float2 vignetteCenter;
    float vignetteIntensity;
    float vignetteSmoothness;

    float vignetteRounded;
    float vignetteRoundness;
    float vignetteOpacity;
    float vignetteDummy;
};

Texture2D sceneMap : register(t0);
SamplerState linearSamplerState : register(s1);

float4 main(VS_OUT pin) : SV_TARGET
{
    float2 sceneMapSize;
    sceneMap.GetDimensions(sceneMapSize.x, sceneMapSize.y);

    float4 color = sceneMap.Sample(linearSamplerState, pin.texcoord);
    
    //  ü•ÓŒ¸Œõˆ—
    float2 d = abs(pin.texcoord - vignetteCenter) * (vignetteIntensity);
    //  Œ¸Œõ‚ğƒXƒNƒŠ[ƒ“‚É‡‚í‚·‚©‚Ç‚¤‚©
    d.x *= lerp(1.0f, sceneMapSize.x / sceneMapSize.y, vignetteRounded);
    //  ‹÷‚Ì”Z‚³
    d = pow(saturate(d), vignetteRoundness);
    half vignetteFactor = pow(saturate(1.0f - dot(d, d)), vignetteSmoothness);
    color.rgb *= lerp(vignetteColor.rgb, (float3) 1.0f, vignetteFactor);
    
    return color;
}

#include "../../Nova/Shaders/FullscreenQuad.hlsli"

#define BASECOLOR_TEXTURE 0
#define METALLIC_ROUGHNESS_TEXTURE 1
#define NORMAL_TEXTURE 2
#define EMISSIVE_TEXTURE 3
#define OCCLUSION_TEXTURE 4

#define LINEAR 1

#define FFT_BLOCK_COUNT 2048

// PROJECTION_MAPPING
SamplerState samplerStates[8] : register(s0);

#define KIND_OF_COLOR 2 //  オーディオスペクトラムの数だけcolorを作る
#define MY_COLOR_INDEX 0

cbuffer fftConstant : register(b10)
{
    float4 data[FFT_BLOCK_COUNT / 4];
    float4 color[KIND_OF_COLOR];
}

float4 main(VS_OUT pin) : SV_TARGET
{
    float2 texcoord = float2(pin.texcoord.x, pin.texcoord.y);
    // quantize coordinates
    const float bands = 30.0;
    const float segs = 40.0;
    float2 p;
    p.x = floor(texcoord.x * bands) / bands;
    p.y = floor(texcoord.y * segs) / segs;

    // read frequency data from first row of texture
    uint t = texcoord.x * FFT_BLOCK_COUNT / 4;
    float4 amp = data[t];
    float offset = texcoord.x * FFT_BLOCK_COUNT;
    float fft = amp[(int) offset % 4];
    
    
    float3 fft_color = float3(1.0, 0.0, 0.0);
    //float3 fft_color = float3(0.8, 0.6, 0.3);
    if (pin.texcoord.y > 0.5)
    {
        float y = (pin.texcoord.y - 0.5) * 2.0;
        //return float4(step(y, fft), 0, 0, 1);
        fft_color *= step(y, fft);
        
    }
    else if (pin.texcoord.y < 0.5)
    {
        float y = (0.5 - pin.texcoord.y) * 2.0;
        //return float4(step(y, fft), 0, 0, 1);
        fft_color *= step(y, fft);

    }
    
    return float4(fft_color, 1.0);
    //return float4(fft_color, 0.5);
    
    
    
    
    //float fft = texture(iChannel0, vec2(p.x, 0.0)).x;
    // led color
    float3 color = lerp(float3(0.0, 2.0, 0.0), float3(2.0, 0.0, 0.0), sqrt(texcoord.y));

    // mask for bar graph
    float mask = (p.y < fft) ? 1.0 : 0.1;

    // led shape
    float2 d = frac((texcoord - p) * float2(bands, segs)) - 0.5;
    float led = smoothstep(0.5, 0.35, abs(d.x)) *
                smoothstep(0.5, 0.35, abs(d.y));
    float3 ledColor = led * color * mask;

    // output final color
    return float4(ledColor, 1.0);
    
    //uint t = pin.texcoord.x * FFT_BLOCK_COUNT / 4;
    //float4 amp = data[t];
    //float offset = pin.texcoord.x * 2048;
    //float v = amp[(int) offset % 4];
    //return float4(v, 0, 0, 1);
    ////float amp = data[pin.position.x / 4];
    
    
    if (pin.position.x % 4 == 0)        amp = data[t].x;
    else if (pin.position.x % 4 == 1)   amp = data[t].y;
    else if (pin.position.x % 4 == 2)   amp = data[t].z;
    else if (pin.position.x % 4 == 3)   amp = data[t].w;
    //return float4(1,0,0, 1);      
    //return float4(amp, amp, amp, 1);
    
    //const float fftColorIntensity = 10.0;
    //const float bands = 30.0;
    //const float segs = 40.0;
    //float3 fftColor = 0;
    //float2 p;
    
    //p.x = pin.texcoord.x;
    //p.y = pin.texcoord.y;

    ////  LEDColor
    //float4 fftTextureColor = float4(lerp(float3(0.0, 2.0, 0.0), float3(2.0, 0.0, 0.0), sqrt(pin.texcoord.y)), 1.0);
    ////float4 fftTextureColor = float4(1, 0, 0, 1);
    //float fft = amp;
    ////return float4(fft, 0, 0, 1);
    
    //// mask for bar graph
    //float mask = (p.y < fft) ? 1.0 : 0.1;
    
    //// led shape
    //float2 d = frac((pin.texcoord - p) * float2(bands, segs)) - 0.5;
    //float led = smoothstep(0.5, 0.35, abs(d.x)) *
    //            smoothstep(0.5, 0.35, abs(d.y));
    //fftTextureColor *= mask;
    ////fftTextureColor *= mask * led * fftColorIntensity;

    //fftColor = fftTextureColor.rgb * fftTextureColor.a * fftColorIntensity ;
    
    //// output final color
    //return float4(fftColor, 1.0);
    ////return 1;
    //return fftTextureColor;

}
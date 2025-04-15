#include "../../Nova/Shaders/FullscreenQuad.hlsli"

#define BASECOLOR_TEXTURE 0
#define METALLIC_ROUGHNESS_TEXTURE 1
#define NORMAL_TEXTURE 2
#define EMISSIVE_TEXTURE 3
#define OCCLUSION_TEXTURE 4

#define LINEAR 1

#define M_PI 3.14159265359
#define FFT_BLOCK_COUNT 2048

#define KIND_OF_COLOR 2 //  オーディオスペクトラムの数だけcolorを作る
#define MY_COLOR_INDEX 1

#define SCREEN_WIDTH  1980
#define SCREEN_HEIGHT 1080

// PROJECTION_MAPPING
SamplerState samplerStates[8] : register(s0);

cbuffer fftConstant : register(b10)
{
    float4 data[FFT_BLOCK_COUNT / 4];
    float4 fftColor[KIND_OF_COLOR];

}

float4 capsule(float4 color, float4 background, float4 region, float2 uv);
float2 rotatePoint(float2 pointA, float2 center, float angle);
float4 bar(float4 color, float4 background, float2 position, float2 diemensions, float2 uv);
float4 rays(float4 color, float4 background, float2 position, float radius, float rays, float ray_length, float fft, float2 uv);

float4 main(VS_OUT pin):SV_TARGET
{
    if (pin.texcoord.x > 0.9)
        return 0;
    if (pin.texcoord.x < 0.1)
        return 0;
    if (pin.texcoord.y > 0.9)
        return 0;
    if (pin.texcoord.y < 0.1)
        return 0;
    //Prepare UV and background
    float aspect = 1.0;
    float2 texcoord = pin.texcoord;
    texcoord.x *= aspect;
#if 0
    float4 color = lerp(float4(0.0, 1.0, 0.8, 1.0), float4(0.0, 0.3, 0.25, 1.0), distance(float2(aspect / 2.0, 0.5), texcoord));
#else
    float4 color = float4(0, 0, 0, 1); //  四角を映さない 
    //float4 color = float4(0.1, 0.2, 0.3, 1.0);  //  ステージと同系色
#endif

    //VISUALIZER PARAMETERS
    const float RAYS = 96.0;    //  number of rays //Please, decrease this value if shader is working too slow
    float RADIUS = 0.4;         //  max circle radius
    float RAY_LENGTH = 0.3;     //  ray's max length //increased by 0.1
    
    //  fft
    uint t = texcoord.x * FFT_BLOCK_COUNT / 4;
    float4 amp = data[t];
    float offset = texcoord.x * FFT_BLOCK_COUNT;
    float fft = amp[(int) offset % 4];
    
    float4 spectrumColor = float4(fftColor[MY_COLOR_INDEX]); //  オーディオスペクトラムの色
    //float4 spectrumColor = float4(0.1, 0.2, 0.3, 1.0);  //  オーディオスペクトラムの色
    color = rays(spectrumColor, color, float2(aspect / 2.0, 1.0 / 2.0), RADIUS, RAYS, RAY_LENGTH, fft, texcoord);

    //return float4(color.xyz, color.a * 0.5);
    return float4(color.xyz, color.a );
}

float4 rays(float4 color, float4 background, float2 position, float radius, float rays, float ray_length, float fft, float2 uv)
{
    float inside = (1.0 - ray_length) * radius; //  empty part of circle
    inside = max(0, inside);
    float outside = radius - inside;            //  rest of circle
    outside = max(0, outside);
    float circle = 2.0 * M_PI * inside;         //  circle lenght
 
    float angleStep = 360.0 / rays; // 各光線の角度間隔
    
    for (int i = 1; float(i) <= rays; i++)
    {
        float len = outside * fft; //    length of actual ray
        background = bar(color, background, float2(position.x, position.y + inside), float2(circle / (rays * 2.0), len), rotatePoint(uv, position, 360.0 / rays * float(i))); //Added capsules
    }
    return background; //output
}

float4 bar(float4 color, float4 background, float2 position, float2 diemensions, float2 uv)
{
    return capsule(color, background, float4(position.x, position.y + diemensions.y / 2.0, diemensions.x / 2.0, diemensions.y / 2.0), uv); //Just transform rectangle a little
}

float4 capsule(float4 color, float4 background, float4 region, float2 uv) //    capsule
{
    if (distance(uv, region.xy - float2(0.0, region.w)) < region.z ||
        distance(uv, region.xy + float2(0.0, region.w)) < region.z)
            return color;

    if (uv.x > (region.x - region.z) && uv.x < (region.x + region.z) &&
        uv.y > (region.y - region.w) && uv.y < (region.y + region.w))
            return color;

    return background;
}

float2 rotatePoint(float2 pointA, float2 center, float angle) //rotating point around the center
{
    float s = sin(radians(angle));
    float c = cos(radians(angle));
    
    pointA.x -= center.x;
    pointA.y -= center.y;
    
    float x = pointA.x * c - pointA.y * s;
    float y = pointA.x * s + pointA.y * c;
    
    pointA.x = x + center.x;
    pointA.y = y + center.y;
    
    return pointA;
}

#include "ImageBasedLighting.hlsli"

#ifndef __BIDIRECTIONAL_REFLECTANCE_DISTRIBUTION_FUNCTION_HLSL__ 
#define __BIDIRECTIONAL_REFLECTANCE_DISTRIBUTION_FUNCTION_HLSL__

float3 FSchlick(float3 f0,float3 f90,float VoH)
{
    return f0 + (f90 - f0) * pow(clamp(1.0f - VoH, 0.0f, 1.0f), 5.0f);
}

float VGgx(float NoL,float NoV,float alphaRoughness)
{
    float alphaRoughnessSq = alphaRoughness * alphaRoughness;
    
    float ggxv = NoL * sqrt(NoV * NoV * (1.0f - alphaRoughnessSq) + alphaRoughnessSq);
    float ggxl = NoV * sqrt(NoL * NoL * (1.0f - alphaRoughnessSq) + alphaRoughnessSq);
    
    float ggx = ggxl + ggxl;
    
    return (ggx > 0.0f) ? 0.5f / ggx : 0.0f;
}

float DGgx(float NoH,float alphaRoughness)
{
    const float PI = 3.14159265358979;
    float alphaRoughnessSq = alphaRoughness * alphaRoughness;
    float f = (NoH * NoH) * (alphaRoughnessSq - 1.0f) + 1.0f;

    return alphaRoughnessSq / (PI * f * f);
}

float3 BrdfLambertian(float3 f0,float3 f90,float3 diffuseColor,float VoH)
{
    const float PI = 3.14159265358979;

    return (1.0f - FSchlick(f0, f90, VoH)) * (diffuseColor / PI);
}

float3 BrdfSpecularGgx(float3 f0,float3 f90,float alphaRoughness,float VoH,float NoL,float NoV,float NoH)
{
    float3 F = FSchlick(f0, f90, VoH);
    float Vis = VGgx(NoL, NoV, alphaRoughness);
    float D = DGgx(NoH, alphaRoughness);
    
    return F * Vis * D;
}

float3 IblRadianceLambertian(float3 N,float3 V,float roughness,float3 diffuseColor,float3 f0)
{
    float NoV = clamp(dot(N, V), 0.0f, 1.0f);
    
    float2 brdfSamplePoint = clamp(float2(NoV, roughness), 0.0f, 1.0f);
    float2 fAb = SampleLutGgx(brdfSamplePoint).rg;
    
    float3 irradiance = SampleDiffuseIem(N).rgb;
    
    float3 fr = max(1.0 - roughness, f0) - f0;
    float3 kS = f0 + fr * pow(1.0f - NoV, 5.0f);
    float3 fssEss = kS * fAb.x + fAb.y;
    
    float ems = (1.0f - (fAb.x + fAb.y));
    float3 fAvg = (f0 * (1.0f - f0) / 21.0f);
    float3 fmsEms = ems * fssEss * fAvg / (1.0f - fAvg * ems);
    float kD = diffuseColor * (1.0f - fssEss + fmsEms);
    
    return (fmsEms + kD) * irradiance;
}

float3 IblRadianceGgx(float3 N,float3 V,float roughness,float3 f0)
{
    float NoV = clamp(dot(N, V), 0.0f, 1.0f);
    
    float2 brdfSamplePoint = clamp(float2(NoV, roughness), 0.0f, 1.0f);
    float2 fAb = SampleLutGgx(brdfSamplePoint).rg;
    
    float3 R = normalize(reflect(-V, N));
    float3 specularLight = SampleSpecularPmrem(R, roughness).rgb;
    
    float3 fr = max(1.0f - roughness, f0) - f0;
    float3 kS = f0 + fr * pow(1.0f - NoV, 5.0f);
    float3 fssEss = kS * fAb.x + fAb.y;
    
    return specularLight * fssEss;
}

#endif
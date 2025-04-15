#include "Model.hlsli"

#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2

SamplerState samplerStates[3] : register(s0);
Texture2D textureMaps[4] : register(t0);

// SHADOW
SamplerComparisonState comparisonSamplerState : register(s5);
Texture2D shadowMap : register(t8);

float4 main(VS_OUT pin) : SV_TARGET
{
    float4 color = textureMaps[0].Sample(samplerStates[ANISOTROPIC], pin.texcoord);
    float alpha = color.a;
    
    //return color;
    
#if 1
    //Inverse gamma process(ãtÉKÉìÉ}ï‚ê≥)
    //const float GAMMA = 2.2f;
    const float GAMMA = 1 / 2.2f;
    color.rgb = pow(color.rgb, GAMMA);
#endif
    
    //TODO:TangentÇÃíl
#if 0 
    float3 N = normalize(pin.worldNormal.xyz);
#else //Shadowï`âÊ
    float3 N = textureMaps[1].Sample(samplerStates[ANISOTROPIC], pin.texcoord).xyz;
#endif
    float3 T = normalize(pin.worldTangent.xyz);
    //float3 T = float3(1.001, 0,0);                                                                                                  
    float sigma = pin.worldTangent.w;
    T = normalize(T - N * dot(N, T));
    //float3 B = normalize(cross(N, T)/* * sigma*/);
    float3 B = normalize(cross(N, T) * sigma);
    
    float4 normal = textureMaps[1].Sample(samplerStates[LINEAR], pin.texcoord);
    //return normal;
    normal = (normal * 2.0) - 1.0;
    N = normalize((normal.x * T) + (normal.y * B) + (normal.z * N));

    float3 L = normalize(-lightDirection.xyz);
    float3 diffuse = color.rgb * max(0, dot(N, L));
    float3 v = normalize(cameraPosition.xyz - pin.worldPosition.xyz);
    float3 specular = pow(max(0, dot(N, normalize(v * L))), 128);
    
    float3 finalColor = diffuse + specular;
    //finalColor.rgb += 0.3f;
    //finalColor.rgb += 0.9f;
    finalColor *= pin.color.rgb;
    
    	// SHADOW
#if 0
	const float shadowDepthBias = 0.001;
#else
	// Here we have a maximum bias of 0.01 and a minimum of 0.001 based on the surface's normal and light direction. 
    const float shadowDepthBias = max(0.01 * (1.0 - dot(N, L)), 0.001);
#endif

    float4 lightViewPosition = mul(pin.worldPosition, lightViewProjection); // World to Clip space
    lightViewPosition = lightViewPosition / lightViewPosition.w; // Clip to NDC
    float2 lightViewTexcoord = 0;
	// NDC to Texture coordinate
    lightViewTexcoord.x = lightViewPosition.x * +0.5 + 0.5;
    lightViewTexcoord.y = lightViewPosition.y * -0.5 + 0.5;
    float depth = saturate(lightViewPosition.z - shadowDepthBias);

    float3 shadowFactor = 1.0f;
    shadowFactor = shadowMap.SampleCmpLevelZero(comparisonSamplerState, lightViewTexcoord, depth).xxx;
    
    float4 emissive = textureMaps[2].Sample(samplerStates[1], pin.texcoord);
    float intencity = 3.0f;
    float3 fColor = diffuse + specular + emissive.xyz * intencity;
    return float4((fColor) * shadowFactor /*SHADOW*/, alpha) * pin.color; //  Shadowï`âÊ x3dgp.shadow
    
#if 0
    return float4(shadowFactor, 1.0f); //  âeÇÃílÇ™éÊÇÍÇƒÇ¢ÇÈÇ©É`ÉFÉbÉN(éÊÇÍÇƒÇ»Ç©Ç¡ÇΩÇÁê^Ç¡îí)
#endif   
    //return float4(pin.color.rgb,1.0f);
    //return float4((diffuse + specular) * shadowFactor /*SHADOW*/, alpha); //  Shadowï`âÊ
    return float4((diffuse + specular) * shadowFactor /*SHADOW*/, alpha) * pin.color; //  Shadowï`âÊ x3dgp.shadow
    //return float4(finalColor, alpha);
}

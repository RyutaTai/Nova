#include "FullScreenQuad.hlsli"

#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
#define LINEAR_BORDER_BLACK 3
#define LINEAR_BORDER_WHITE 4

SamplerState	samplerStates[5]	: register(s0);
Texture2D		textureMaps[2]		: register(t0);
Texture2D		depthMap			: register(t2);
Texture2DArray	cascadedShadowMaps	: register(t3); //	シャドウ

float3 ReinhardToneMapping(float3 color)
{
	float luma = dot(color, float3(0.2126, 0.7152, 0.0722));
	float toneMappedLuma = luma / (1. + luma);
	color *= toneMappedLuma / luma;
	return color;
}

cbuffer SCENE_CONSTANT_BUFFER : register(b1)
{
    row_major float4x4 viewProjection;
    float4 lightDirection;
    float4 cameraPosition;
    row_major float4x4 lightViewProjection;
    row_major float4x4 invViewProjection;
};

float4 main(VS_OUT pin) : SV_TARGET
{
//	//	ここから(bloomの前に入れる)
//	// begin*********************************
//    float depthNdc = depthMap.Sample(samplerStates[LINEAR_BORDER_BLACK], pin.texcoord).x;

//    float4 positionNdc;
//	// texture space to ndc
//    positionNdc.x = pin.texcoord.x * +2 - 1;
//    positionNdc.y = pin.texcoord.y * -2 + 1;
//    positionNdc.z = depthNdc;
//    positionNdc.w = 1;

//	// ndc to view space
//    float4 positionViewSpace = mul(positionNdc, invProjection);
//    positionViewSpace = positionViewSpace / positionViewSpace.w;
	
//	// ndc to world space
//    float4 positionWorldSpace = mul(positionNdc, invViewProjection);
//    positionWorldSpace = positionWorldSpace / positionWorldSpace.w;

	
	
//	// Apply cascaded shadow mapping
//	// Find a layer of cascaded view frustum volume 
//    float depthViewSpace = positionViewSpace.z;
//    int cascadeIndex = -1;
//    for (uint layer = 0; layer < 4; ++layer)
//    {
//        float distance = cascadedPlaneSistances[layer];
//        if (distance > depthViewSpace)
//        {
//            cascadeIndex = layer;
//            break;
//        }
//    }
//    float shadowFactor = 1.0;
//    if (cascadeIndex > -1)
//    {
//		// world space to light view clip space, and to ndc
//        float4 positionLightSpace = mul(positionWorldSpace, cascadedMatrices[cascadeIndex]);
//        positionLightSpace /= positionLightSpace.w;
//		// ndc to texture space
//        positionLightSpace.x = positionLightSpace.x * +0.5 + 0.5;
//        positionLightSpace.y = positionLightSpace.y * -0.5 + 0.5;
	
//        shadowFactor = cascadedShadowMaps.SampleCmpLevelZero(comparisonSamplerState, float3(positionLightSpace.xy, cascadeIndex), positionLightSpace.z - shadowDepthBias).x;
	
//        float3 layerColor = 1;
//#if 1
//        if (colorizeCascadedLayer)
//        {
//            const float3 layerColors[4] =
//            {
//                { 1, 0, 0 },
//                { 0, 1, 0 },
//                { 0, 0, 1 },
//                { 1, 1, 0 },
//            };
//            layerColor = layerColors[cascadeIndex];
//        }
//#endif
//        color *= lerp(shadowColor, 1.0, shadowFactor) * layerColor;
//    }
	
//	// end*******************************
	
	float4 color = textureMaps[0].Sample(samplerStates[POINT], pin.texcoord);
	float4 bloom = textureMaps[1].Sample(samplerStates[POINT], pin.texcoord);

	float3 fragmentColor = color.rgb + bloom.rgb;
	float alpha = color.a;

	// Tone map
	fragmentColor = ReinhardToneMapping(fragmentColor);

	// Gamma correction
	const float INV_GAMMA = 1.0 / 2.2;
	fragmentColor = pow(fragmentColor, INV_GAMMA);

	return float4(fragmentColor, alpha);
}

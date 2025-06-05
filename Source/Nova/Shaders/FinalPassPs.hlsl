#include "FullScreenQuad.hlsli"

#include "ColorFilter.hlsli"
#include "ChromaticAberration.hlsli"

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

float4 main(VS_OUT pin) : SV_TARGET
{	
    float2 baseTexcoord = pin.texcoord;

    // 各色チャンネルを異なるUV座標でサンプリング
    // 強度 (chromaticAberrationStrength)とオフセットの方向を調整
    // 例えば、赤は左上、青は右下へ少しずらす
    float4 colorR = textureMaps[0].Sample(samplerStates[POINT], baseTexcoord - chromaticAberrationStrength);
    float4 colorG = textureMaps[0].Sample(samplerStates[POINT], baseTexcoord); // 緑はそのまま
    float4 colorB = textureMaps[0].Sample(samplerStates[POINT], baseTexcoord + chromaticAberrationStrength);

    // 各チャンネルを合成して最終的な色を構築
    float4 color = float4(colorR.r, colorG.g, colorB.b, colorR.a); // アルファはどれか一つから取得

    
    //  ブルーム
	float4 bloom = textureMaps[1].Sample(samplerStates[POINT], pin.texcoord);
	float3 fragmentColor = color.rgb + bloom.rgb;
	float alpha = color.a;

	//  トーンマップ
	fragmentColor = ReinhardToneMapping(fragmentColor);

    //  カラーフィルター
    {
        // RGB > HSVに変換
        fragmentColor.rgb = RGB2HSV(fragmentColor.rgb);

	    // 色相調整
        fragmentColor.r += hueShift;

	    // 彩度調整
        fragmentColor.g *= saturation;

	    // 明度調整
        fragmentColor.b *= brightness;

	    // HSV > RGBに変換
        fragmentColor.rgb = HSV2RGB(fragmentColor.rgb);

    }
    
    //  周辺減光(ヴィネット)
    {
	    //  シーンマップのサイズ取得
        float2 sceneMapSize;
        textureMaps[0].GetDimensions(sceneMapSize.x, sceneMapSize.y);

        //  周辺減光処理
        float2 d = abs(pin.texcoord - vignetteCenter) * (vignetteIntensity);
        //  減光をスクリーンに合わすかどうか
        d.x *= lerp(1.0f, sceneMapSize.x / sceneMapSize.y, vignetteRounded);
        //  隅の濃さ
        d = pow(saturate(d), vignetteRoundness);
        half vignetteFactor = pow(saturate(1.0f - dot(d, d)), vignetteSmoothness);
        fragmentColor.rgb *= lerp(vignetteColor.rgb, (float3)1.0f, vignetteFactor);
    }
        
	// Gamma correction
    const float INV_GAMMA = 1.0 / 2.2;
    fragmentColor = pow(fragmentColor, INV_GAMMA);

	return float4(fragmentColor, alpha);
}

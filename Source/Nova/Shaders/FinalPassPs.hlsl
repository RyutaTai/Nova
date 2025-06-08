#include "FullScreenQuad.hlsli"

#include "ColorFilter.hlsli"

#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
#define LINEAR_BORDER_BLACK 3
#define LINEAR_BORDER_WHITE 4

SamplerState	samplerStates[5]	: register(s0);
Texture2D		textureMaps[2]		: register(t0);
Texture2D		depthMap			: register(t2);
Texture2DArray	cascadedShadowMaps	: register(t3); //	シャドウ

//  色収差の強さを制御する定数
cbuffer ChromaticAberrationConstantBuffer : register(b6)
{
    float2 chromaticAberrationStrength; // x成分で水平方向、y成分で垂直方向のずれ
};

//  ヴィネット用定数バッファ
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

//  シャープネスフィルター用定数バッファ
cbuffer SharpenConstantBuffer : register(b9)
{
    float  sharpenAmount;   //  シャープネスの強さ
    float3 dummySharpen;    //  パディング
};

//  コントラスト用定数バッファ
cbuffer ContrastConstantBuffer : register(b13)
{
    float   contrast;       //  コントラストの強さ (1.0が標準)
    float3  dummyContrast;  //  パディング
};

//  露出フィルター用定数バッファ
cbuffer ExposureConstantBuffer : register(b8)
{
    float   exposure;      //   露出の強さ
    float3  dummyExposure; //   パディング
};

//  トーンマップ用
float3 ReinhardToneMapping(float3 color)
{
    float luma = dot(color, float3(0.2126, 0.7152, 0.0722));
    float toneMappedLuma = luma / (1. + luma);
    color *= toneMappedLuma / luma;
    return color;
}

//  乱数生成関数
//  Texcoordと時間に基づいてシードを生成し、一様乱数を返す
float Rand(float2 co,float time)
{
    return frac(sin(dot(co.xy, float2(12.9898, 78.233) + time)) * 43758.5453);
}

//  コントラスト調整用
//  シンプルなS字カーブによるコントラスト調整
//  input: 0.0から1.0の範囲の色
//  contrast: 0.0でコントラストなし、1.0で標準、1.0より大きくするとコントラストが強くなる
float3 AdjustContrast(float3 color, float contrastAmount)
{
    //  グレースケールに変換し、中間点からの差を計算する一般的な方法
    float3 lumaCoeff = float3(0.2126, 0.7152, 0.0722);
    float averageLuma = dot(color, lumaCoeff);

    //  中間点 (0.5) を基準にコントラストを適用
    //  contrastAmountは通常、1.0を中心として調整
    //  0.0から1.0の範囲にクランプする
    return saturate((color - 0.5) * contrastAmount + 0.5);
}

float4 main(VS_OUT pin) : SV_TARGET
{	
    float2 baseTexcoord = pin.texcoord;
    float4 originalColor = textureMaps[0].Sample(samplerStates[POINT], baseTexcoord);
    
    //  色収差
    //  各色チャンネルを異なるUV座標でサンプリング
    //  強度 (chromaticAberrationStrength)とオフセットの方向を調整
    //  例えば、赤は左上、青は右下へ少しずらす
    float4 colorR = textureMaps[0].Sample(samplerStates[POINT], baseTexcoord - chromaticAberrationStrength);
    float4 colorG = textureMaps[0].Sample(samplerStates[POINT], baseTexcoord); // 緑はそのまま
    float4 colorB = textureMaps[0].Sample(samplerStates[POINT], baseTexcoord + chromaticAberrationStrength);

    //  各チャンネルを合成して最終的な色を構築
    float4 color = float4(colorR.r, colorG.g, colorB.b, colorR.a); // アルファはどれか一つから取得
    
    //  ブルーム
	float4 bloom = textureMaps[1].Sample(samplerStates[POINT], pin.texcoord);
	float3 fragmentColor = color.rgb + bloom.rgb;
	float alpha = color.a;

    //  露出の適用
    fragmentColor *= exposure;
    
	//  トーンマップ(Reinhard)
	fragmentColor = ReinhardToneMapping(fragmentColor);

    //  コントラスト調整
    fragmentColor = AdjustContrast(fragmentColor, contrast);
    
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
    
    //  シャープネスフィルター
    {
        //  テクセルサイズを計算
        float2 sceneMapSize;
        textureMaps[0].GetDimensions(sceneMapSize.x, sceneMapSize.y);
        float2 texelSize = 1.0 / sceneMapSize;
        
        //  3x3 Laplacian sharpening kernel (エッジ強調)
        //  中央ピクセルを強調し、周囲を減算する
        float3 sharpenResult = fragmentColor.rgb * (1.0 + 4.0 * sharpenAmount); // 中央を強調
        sharpenResult -= textureMaps[0].Sample(samplerStates[POINT], baseTexcoord + float2(0, texelSize.y)).rgb * sharpenAmount;
        sharpenResult -= textureMaps[0].Sample(samplerStates[POINT], baseTexcoord - float2(0, texelSize.y)).rgb * sharpenAmount;
        sharpenResult -= textureMaps[0].Sample(samplerStates[POINT], baseTexcoord + float2(texelSize.x, 0)).rgb * sharpenAmount;
        sharpenResult -= textureMaps[0].Sample(samplerStates[POINT], baseTexcoord - float2(texelSize.x, 0)).rgb * sharpenAmount;
        
        //  結果を元の色にブレンド (saturateで0～1にクランプ)
        fragmentColor.rgb = saturate(sharpenResult);
        
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
    
	//  ガンマ補正
    const float INV_GAMMA = 1.0 / 2.2;
    fragmentColor = pow(fragmentColor, INV_GAMMA);

	return float4(fragmentColor, alpha);
}

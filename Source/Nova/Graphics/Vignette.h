#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <directxmath.h>

//	周辺減光(ヴィネット)
class Vignette
{
public:
	//	ヴィネット用パラメータ(CPU側でのみ使用するパラメータも含む)
	struct VignetteData
	{
		DirectX::XMFLOAT4	vignetteColor_		= { 0.2f, 0.2f, 0.2f, 1.0f };	//	カラー
		DirectX::XMFLOAT2	vignetteCenter_		= { 0.5f, 0.5f };				//	注視点(どこを中心にするか)

		float				vignetteCurrentIntensity_ = 0.5f;					//	現在の範囲(大きさ)
		float				vignetteIntensityMax_ = 0.552f;						//	範囲最大値
		float				vignetteIntensityMin_ = 0.4f;						//	範囲最小値

		float				vignetteSmoothness_ = 0.4f;							//	どのくらいぼかすか
		bool				vignetteRounded_	= false;						//	縦横比を等しくするかどうか(trueなら縦横比が同じになる)
		float				vignetteRoundness_	= 0.63f;						//	形の調整(0に近づくほど四角く、1に近づくほど丸くなる)
	};
	VignetteData vignetteData_;

	//	ヴィネット用定数バッファ
	struct VignetteConstants
	{
		DirectX::XMFLOAT4	vignetteColor_ = {};
		DirectX::XMFLOAT2	vignetteCenter_ = {};
		float				vignetteIntensity_ = 0.0f;
		float				vignetteSmoothness_ = 0.0f;

		float				vignetteRounded_ = 0.0f;
		float				vignetteRoundness_ = 0.0f;
		DirectX::XMFLOAT2	vignetteDummy_ = {};
	};

public:
	Vignette();
	~Vignette() = default;

	static Vignette& Instance()
	{
		static Vignette instance;
		return instance;
	}

	void Make();
	void DrawDebug();

	//	ヴィネットの範囲
	void SetVignetteIntensity(const float& intensity) { vignetteData_.vignetteCurrentIntensity_ = intensity; }
	const float GetVignetteCurrentIntensity()	const { return vignetteData_.vignetteCurrentIntensity_; }
	const float GetVignetteIntensityMax()		const { return vignetteData_.vignetteIntensityMax_; }
	const float GetVignetteIntensityMin()		const { return vignetteData_.vignetteIntensityMin_; }

	void LerpVignetteIntensity(const float& elapsedTime);
	void SetIsFadeIn(const bool& isFadeIn) { isFadeIn_ = isFadeIn; }
	void SetLerpFlag(const bool& isLerp) { isLerp_ = isLerp; }

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer>		vignetteConstantBuffer_;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	vignettePixelShader_;

	float lerpTimerMax_ = 0.17f;
	float lerpTimer_ = 0.0f;
	bool isFadeIn_ = false;
	bool isLerp_ = false;

};


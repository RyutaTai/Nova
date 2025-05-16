#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <directxmath.h>

//	周辺減光(ヴィネット)
class Vignette
{
public:
	struct VignetteData
	{
		DirectX::XMFLOAT4	vignetteColor_		= { 0.2f, 0.2f, 0.2f, 1.0f };
		DirectX::XMFLOAT2	vignetteCenter_		= { 0.5f, 0.5f };
		float				vignetteIntensity_	= 0.5f;
		float				vignetteSmoothness_ = 0.2f;

		bool				vignetteRounded_	= false;
		float				vignetteRoundness_	= 0.2f;
	};
	VignetteData vignetteData_;

	struct VignetteConstants
	{
		DirectX::XMFLOAT4	vignetteColor_;
		DirectX::XMFLOAT2	vignetteCenter_;
		float				vignetteIntensity_;
		float				vignetteSmoothness_;

		float				vignetteRounded_;
		float				vignetteRoundness_;
		DirectX::XMFLOAT2	vignetteDummy_;
	};

public:
	Vignette();
	virtual ~Vignette() = default;
	void Make();
	void DrawDebug();

	ID3D11PixelShader* GetVignettePixelShader() { return vignettePixelShader_.Get(); }

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer>		vignetteConstantBuffer_;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	vignettePixelShader_;

};


#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <directxmath.h>

//	カラーフィルター
class ColorFilter
{
public:
	ColorFilter();
	virtual ~ColorFilter() = default;

	void Update();
	void DrawDebug();

private:
	//	カラーフィルター用定数バッファ
	struct ColorFilterConstants
	{
		float	hueShift_ = 0.1f;		//	色相調整
		float	saturation_ = 1.0f;		//	彩度調整
		float	brightness_ = 1.5f;		//	明度調整
		float	dummy_ = 0.0f;
	};
	ColorFilterConstants colorFilterConstant_;
	Microsoft::WRL::ComPtr<ID3D11Buffer> colorFilterConstantBuffer_;

};


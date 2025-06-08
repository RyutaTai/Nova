#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <directxmath.h>

//  露出フィルター
class ExposureFilter
{
public:
    struct ExposureConstants
    {
		float               exposure_ = 1.5f;   //    露出の強さ
        DirectX::XMFLOAT3   dummy_;             //    パディング
    };
    ExposureConstants constants_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer_;

public:
    ExposureFilter();
    virtual ~ExposureFilter() = default;

    void Update();
    void DrawDebug();

};
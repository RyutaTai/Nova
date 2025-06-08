#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <directxmath.h>

//  シャープネスフィルター
class SharpenFilter
{
public:
    struct SharpenConstants
    {
        float               sharpenAmount_;
        DirectX::XMFLOAT3   dummy_;         //  パディング
    };
    SharpenConstants constants_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer_;

public:
    SharpenFilter();
    virtual ~SharpenFilter() = default;

    void Update();
    void DrawDebug();

};
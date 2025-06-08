#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <directxmath.h>

//  グレインノイズフィルター
class GrainNoiseFilter
{
public:
    struct GrainNoiseConstants
    {
        float               grainStrength_;
        DirectX::XMFLOAT2   dummy_;         //  パディング
        float               time_;          //  時間
    };
    GrainNoiseConstants constants_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer_;

public:
    GrainNoiseFilter();
    virtual ~GrainNoiseFilter() = default;

    void Update(const float& elapsedTime); // 時間を渡すためにelapsedTimeを受け取る
    void DrawDebug();

};
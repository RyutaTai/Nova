#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <directxmath.h>

//  コントラストフィルター
class ContrastFilter
{
public:
    ContrastFilter();
    virtual ~ContrastFilter() = default;

    void Update();
    void DrawDebug();

private:
    struct ContrastConstants
    {
        float               contrast_ = 1.0f;   //  コントラストの強さ(1.0が標準)
        DirectX::XMFLOAT3   dummy_;             //  パディング
    };
    ContrastConstants constants_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer_;

};
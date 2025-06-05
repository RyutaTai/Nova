#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <directxmath.h>

//  色収差
class ChromaticAberration
{
public:
    struct ChromaticAberrationConstants
    {
        DirectX::XMFLOAT2   strength_   = {};       //  色収差の強さ (X:水平 Y:垂直)
        DirectX::XMFLOAT2   dummy_      = {};       //  パディング
    };
    ChromaticAberrationConstants chromaticAberrationConstant_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> chromaticAberrationConstantBuffer_;

public:
    ChromaticAberration();
    ~ChromaticAberration() = default;

    void Update();
    void DrawDebug(); // ImGui用

};
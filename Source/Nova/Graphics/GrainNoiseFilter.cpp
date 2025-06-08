#include "GrainNoiseFilter.h"

#include "../../Nova/Others/Misc.h"
#include "../../imgui/imgui.h"
#include "../../Nova/Graphics/Graphics.h"

GrainNoiseFilter::GrainNoiseFilter()
{
    HRESULT hr = S_OK;

    //  定数バッファの生成
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;
    bufferDesc.ByteWidth = sizeof(GrainNoiseConstants);
    hr = Graphics::Instance().GetDevice()->CreateBuffer(&bufferDesc, nullptr, constantBuffer_.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

    //  初期値設定
    constants_.grainStrength_ = 0.05f; // デフォルトの強さ
    constants_.time_ = 0.0f;

}

void GrainNoiseFilter::Update(const float& elapsedTime)
{
    //  時間を更新 (連続的なノイズのアニメーション用)
    constants_.time_ += elapsedTime;

    //  定数バッファの更新とシェーダーへのセット
    static constexpr int GrainNoiseCBIndex = 8;
    Graphics::Instance().GetDeviceContext()->UpdateSubresource(constantBuffer_.Get(), 0, 0, &constants_, 0, 0);
    Graphics::Instance().GetDeviceContext()->PSSetConstantBuffers(GrainNoiseCBIndex, 1, constantBuffer_.GetAddressOf());

}

void GrainNoiseFilter::DrawDebug()
{
    if (ImGui::TreeNode("GrainNoise"))
    {
        ImGui::SliderFloat("Strength", &constants_.grainStrength_, 0.0f, 0.5f); // 調整可能な範囲
        ImGui::TreePop();
    }
}
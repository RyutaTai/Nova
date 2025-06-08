#include "ExposureFilter.h"

#include "../../Nova/Others/Misc.h"      // _ASSERT_EXPR, HRTrace のために必要
#include "../../Nova/Graphics/Graphics.h" // Graphics::Instance().GetDevice() のために必要
#include "../../imgui/imgui.h"           // ImGuiデバッグ描画用

ExposureFilter::ExposureFilter()
{
    HRESULT hr = S_OK;

    //  定数バッファの生成
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;
    bufferDesc.ByteWidth = sizeof(ExposureConstants); // 構造体サイズをそのまま使用
    hr = Graphics::Instance().GetDevice()->CreateBuffer(&bufferDesc, nullptr, constantBuffer_.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

}

void ExposureFilter::Update()
{
    static constexpr int ExposureCBIndex = 8;
    Graphics::Instance().GetDeviceContext()->UpdateSubresource(constantBuffer_.Get(), 0, 0, &constants_, 0, 0);
    Graphics::Instance().GetDeviceContext()->PSSetConstantBuffers(ExposureCBIndex, 1, constantBuffer_.GetAddressOf());

}

void ExposureFilter::DrawDebug()
{
    if (ImGui::TreeNode(u8"ExposureFilter 露出"))
    {
        //  露出の調整範囲。0.0は真っ暗、1.0は標準、それ以上は明るくなる
        ImGui::SliderFloat("Exposure", &constants_.exposure_, 0.0f, 5.0f);
        ImGui::TreePop();
    }
}
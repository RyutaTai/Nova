#include "SharpenFilter.h"
#include "../../Nova/Others/Misc.h"
#include "../../imgui/imgui.h"
#include "../../Nova/Graphics/Graphics.h"

SharpenFilter::SharpenFilter()
{
    HRESULT hr = S_OK;

    //  定数バッファの生成
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;
    bufferDesc.ByteWidth = sizeof(SharpenConstants);
    hr = Graphics::Instance().GetDevice()->CreateBuffer(&bufferDesc, nullptr, constantBuffer_.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

}

void SharpenFilter::Update()
{
    static constexpr int SharpenCBIndex = 9;
    Graphics::Instance().GetDeviceContext()->UpdateSubresource(constantBuffer_.Get(), 0, 0, &constants_, 0, 0);
    Graphics::Instance().GetDeviceContext()->PSSetConstantBuffers(SharpenCBIndex, 1, constantBuffer_.GetAddressOf());
}

void SharpenFilter::DrawDebug()
{
    if (ImGui::TreeNode(u8"SharpenFilter シャープネス"))
    {
        ImGui::SliderFloat("Amount", &constants_.sharpenAmount_, 0.0f, 0.1f); // 調整可能な範囲 (シャープは強すぎるとノイズになるので控えめに)
        ImGui::TreePop();
    }
}
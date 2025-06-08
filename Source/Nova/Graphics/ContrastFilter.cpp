#include "ContrastFilter.h"

#include "../../Nova/Others/Misc.h"
#include "../../Nova/Graphics/Graphics.h"
#include "../../imgui/imgui.h"

ContrastFilter::ContrastFilter()
{
    HRESULT hr = S_OK;

    //  定数バッファの生成
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;
    bufferDesc.ByteWidth = sizeof(ContrastConstants);
    hr = Graphics::Instance().GetDevice()->CreateBuffer(&bufferDesc, nullptr, constantBuffer_.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

}

void ContrastFilter::Update()
{
    static constexpr int ContrastCBIndex = 13;
    Graphics::Instance().GetDeviceContext()->UpdateSubresource(constantBuffer_.Get(), 0, 0, &constants_, 0, 0);
    Graphics::Instance().GetDeviceContext()->PSSetConstantBuffers(ContrastCBIndex, 1, constantBuffer_.GetAddressOf());
}

void ContrastFilter::DrawDebug()
{
    if (ImGui::TreeNode(u8"ContrastFilter コントラスト"))
    {
        //  コントラストの調整範囲
        //  1.0が標準。0.0で単色に近づく、1.0より大きくするとコントラストが強くなる
        ImGui::SliderFloat("Contrast", &constants_.contrast_, 0.0f, 3.0f);  //  0.0から3.0の範囲で調整可能
        ImGui::TreePop();
    }
}
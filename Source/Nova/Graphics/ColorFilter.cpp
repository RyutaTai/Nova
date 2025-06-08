#include "ColorFilter.h"

#include "../../Nova/Others/Misc.h"
#include "../../Nova/Graphics/Graphics.h"
#include "../../imgui/imgui.h"

ColorFilter::ColorFilter()
{
	HRESULT hr = S_OK;

	//	定数バッファの生成
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;
	bufferDesc.ByteWidth = sizeof(ColorFilterConstants);
	hr = Graphics::Instance().GetDevice()->CreateBuffer(&bufferDesc, nullptr, colorFilterConstantBuffer_.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

}

void ColorFilter::Update()
{
	static constexpr int ColorFilterCBIndex = 7;
	Graphics::Instance().GetDeviceContext()->UpdateSubresource(colorFilterConstantBuffer_.Get(), 0, 0, &colorFilterConstant_, 0, 0);
	Graphics::Instance().GetDeviceContext()->PSSetConstantBuffers(ColorFilterCBIndex, 1, colorFilterConstantBuffer_.GetAddressOf());

}

void ColorFilter::DrawDebug()
{
	if (ImGui::TreeNode(u8"ColorFilter カラーフィルター"))
	{
		ImGui::SliderFloat("HueShift", &colorFilterConstant_.hueShift_, 0.1f, +359.0f);
		ImGui::SliderFloat("Saturation", &colorFilterConstant_.saturation_, 0.0f, +2.0f);
		ImGui::SliderFloat("Brightness", &colorFilterConstant_.brightness_, 0.0f, +2.0f);

		ImGui::TreePop();
	}
}
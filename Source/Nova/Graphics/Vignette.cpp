#include "Vignette.h"

#include "../../Nova/Others/Misc.h"
#include "../../imgui/imgui.h"
#include "Graphics.h"
#include "Shader.h"

Vignette::Vignette()
{
	HRESULT hr = S_OK;

	//	ヴィネット用定数バッファの生成
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;
	bufferDesc.ByteWidth = sizeof(VignetteConstants);
	hr = Graphics::Instance().GetDevice()->CreateBuffer(&bufferDesc, nullptr, vignetteConstantBuffer_.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	//	ヴィネット用ピクセルシェーダー
	Graphics::Instance().GetShader()->CreatePsFromCso(Graphics::Instance().GetDevice(), "./Resources/Shader/VignettePS.cso", vignettePixelShader_.GetAddressOf());

}

void Vignette::Make()
{
	//	ヴィネット用定数バッファ
	static constexpr int VignetteCBVIndex = 2;
	VignetteConstants constant;
	constant.vignetteColor_		= vignetteData_.vignetteColor_;
	constant.vignetteCenter_	= vignetteData_.vignetteCenter_;
	constant.vignetteIntensity_ = vignetteData_.vignetteCurrentIntensity_ * 3.0f;
	constant.vignetteSmoothness_ = max(0.000001f, vignetteData_.vignetteSmoothness_ * 5.0f);
	constant.vignetteRounded_	= vignetteData_.vignetteRounded_ ? 1.0f : 0.0f;
	constant.vignetteRoundness_ = 6.0f * (1.0f - vignetteData_.vignetteRoundness_) + vignetteData_.vignetteRoundness_;

	Graphics::Instance().GetDeviceContext()->UpdateSubresource(vignetteConstantBuffer_.Get(), 0, 0, &constant, 0, 0);
	Graphics::Instance().GetDeviceContext()->PSSetConstantBuffers(VignetteCBVIndex, 1, vignetteConstantBuffer_.GetAddressOf());

}

void Vignette::DrawDebug()
{
	if (ImGui::TreeNode("Vignette"))
	{
		ImGui::ColorEdit3("Color", &vignetteData_.vignetteColor_.x);
		ImGui::SliderFloat2("Center", &vignetteData_.vignetteCenter_.x, 0, 1);
		ImGui::SliderFloat("CurrentIntensity", &vignetteData_.vignetteCurrentIntensity_, 0.0f, +1.0f);
		ImGui::SliderFloat("MaxIntensity", &vignetteData_.vignetteIntensityMax_, 0.0f, +1.0f);
		ImGui::SliderFloat("MinIntensity", &vignetteData_.vignetteIntensityMin_, 0.0f, +1.0f);
		ImGui::SliderFloat("Smoothness", &vignetteData_.vignetteSmoothness_, 0.0f, +1.0f);
		ImGui::Checkbox("Rounded", &vignetteData_.vignetteRounded_);
		ImGui::SliderFloat("Roundness", &vignetteData_.vignetteRoundness_, 0.0f, +1.0f);

		ImGui::TreePop();
	}
}

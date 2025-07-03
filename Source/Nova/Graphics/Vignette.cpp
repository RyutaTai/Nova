#include "Vignette.h"

#include "../../Nova/Others/Misc.h"
#include "../../imgui/imgui.h"
#include "Graphics.h"
#include "Shader.h"
#include "../../Nova/Others/MathHelper.h"

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
	constant.vignetteIntensity_ = vignetteData_.currentVignetteIntensity_ * 3.0f;
	constant.vignetteSmoothness_ = max(0.000001f, vignetteData_.vignetteSmoothness_ * 5.0f);
	constant.vignetteRounded_	= vignetteData_.vignetteRounded_ ? 1.0f : 0.0f;
	constant.vignetteRoundness_ = 6.0f * (1.0f - vignetteData_.vignetteRoundness_) + vignetteData_.vignetteRoundness_;

	Graphics::Instance().GetDeviceContext()->UpdateSubresource(vignetteConstantBuffer_.Get(), 0, 0, &constant, 0, 0);
	Graphics::Instance().GetDeviceContext()->PSSetConstantBuffers(VignetteCBVIndex, 1, vignetteConstantBuffer_.GetAddressOf());

}

void Vignette::LerpVignetteIntensity(const float& elapsedTime)
{
	//	補完フラグが経っていなければ更新しない
	if (vignetteIsLerp_ == false)return;

	//	補完タイマー更新
	vignetteLerpTimer_ += elapsedTime;
	
	//	補完しきったらリセット
	if (vignetteLerpTimer_ >= vignetteLerpTimerMax_)
	{
		vignetteIsFadeIn_ = !vignetteIsFadeIn_;
		if (vignetteIsFadeIn_ == false)vignetteIsLerp_ = false;
		vignetteLerpTimer_ = 0.0f;
	}

	//	ヴィネットの強度を強めるか弱めるかで補完する最大値、最小値を切り替える
	if (vignetteIsFadeIn_)
	{
		vignetteData_.currentVignetteIntensity_ = 
			Mathf::Lerp(vignetteData_.vignetteIntensityMin_, vignetteData_.vignetteIntensityMax_, vignetteLerpTimer_ / vignetteLerpTimerMax_);
	}
	else
	{
		vignetteData_.currentVignetteIntensity_ =
			Mathf::Lerp(vignetteData_.vignetteIntensityMax_, vignetteData_.vignetteIntensityMin_, vignetteLerpTimer_ / vignetteLerpTimerMax_);
	}
}

void Vignette::DrawDebug()
{
	if (ImGui::TreeNode(u8"Vignette ヴィネット"))
	{
		ImGui::ColorEdit3("Color", &vignetteData_.vignetteColor_.x);
		ImGui::SliderFloat2("Center", &vignetteData_.vignetteCenter_.x, 0, 1);
		ImGui::SliderFloat("CurrentIntensity", &vignetteData_.currentVignetteIntensity_, 0.0f, +1.0f);
		ImGui::SliderFloat("MaxIntensity", &vignetteData_.vignetteIntensityMax_, 0.0f, +1.0f);
		ImGui::SliderFloat("MinIntensity", &vignetteData_.vignetteIntensityMin_, 0.0f, +1.0f);
		ImGui::SliderFloat("Smoothness", &vignetteData_.vignetteSmoothness_, 0.0f, +1.0f);
		ImGui::Checkbox("Rounded", &vignetteData_.vignetteRounded_);
		ImGui::SliderFloat("Roundness", &vignetteData_.vignetteRoundness_, 0.0f, +1.0f);

		//	補完に使用する変数
		ImGui::DragFloat("LerpTimer", &vignetteLerpTimer_, 0.01f);
		ImGui::DragFloat("LerpTimerMax", &vignetteLerpTimerMax_, 0.01f);
		ImGui::Checkbox("IsLerp", &vignetteIsLerp_);
		ImGui::Checkbox("IsFadeIn", &vignetteIsFadeIn_);

		ImGui::TreePop();
	}
}

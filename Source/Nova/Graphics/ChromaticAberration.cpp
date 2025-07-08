#include "ChromaticAberration.h"

#include "../../Nova/Others/Misc.h"
#include "../../Nova/Graphics/Graphics.h"
#include "../../imgui/imgui.h"

ChromaticAberration::ChromaticAberration()
{
	HRESULT hr = S_OK;

	//	色収差用定数バッファの生成
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;
	bufferDesc.ByteWidth = sizeof(ChromaticAberrationConstants);
	hr = Graphics::Instance().GetDevice()->CreateBuffer(&bufferDesc, nullptr, chromaticAberrationConstantBuffer_.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

}

void ChromaticAberration::Update()
{
	static constexpr int ChromaticAberrationCBIndex = 6; //	色収差用定数バッファのレジスタ番号
	Graphics::Instance().GetDeviceContext()->UpdateSubresource(chromaticAberrationConstantBuffer_.Get(), 0, 0, &chromaticAberrationConstant_, 0, 0);
	Graphics::Instance().GetDeviceContext()->PSSetConstantBuffers(ChromaticAberrationCBIndex, 1, chromaticAberrationConstantBuffer_.GetAddressOf());
}

void ChromaticAberration::DrawDebug()
{
	if (ImGui::TreeNode(u8"ChromaticAberration 色収差"))
	{
		ImGui::SliderFloat2("Strength", &chromaticAberrationConstant_.strength_.x, 0.0f, 0.01f); // 適切な範囲に調整
		ImGui::TreePop();
	}
}

#include "FullScreenQuad.h"

#include "Shader.h"
#include "../Others/Misc.h"
#include "../Graphics/Graphics.h"

//	コンストラクタ
FullScreenQuad::FullScreenQuad(ID3D11Device* device)
{
	Graphics::Instance().GetShader()->CreateVsFromCso(device, "./Resources/Shader/FullScreenQuadVs.cso", embeddedVertexShader_.ReleaseAndGetAddressOf(),
		nullptr, nullptr, 0);
	Graphics::Instance().GetShader()->CreatePsFromCso(device, "./Resources/Shader/FullScreenQuadPs.cso", embeddedPixelShader_.ReleaseAndGetAddressOf());
}

void FullScreenQuad::Blit(ID3D11DeviceContext* deviceContext,
	ID3D11ShaderResourceView** shaderResourceView, uint32_t startSlot, uint32_t numViews,
	ID3D11PixelShader* replacedPixelShader)
{
	deviceContext->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	deviceContext->IASetInputLayout(nullptr);

	deviceContext->VSSetShader(embeddedVertexShader_.Get(), 0, 0);
	replacedPixelShader ? deviceContext->PSSetShader(replacedPixelShader, 0, 0) :
		deviceContext->PSSetShader(embeddedPixelShader_.Get(), 0, 0);

	deviceContext->PSSetShaderResources(startSlot, numViews, shaderResourceView);

	deviceContext->Draw(4, 0);
}
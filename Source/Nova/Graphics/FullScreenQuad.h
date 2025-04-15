#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <cstdint>

class FullScreenQuad
{
public:
	FullScreenQuad(ID3D11Device* device);
	virtual ~FullScreenQuad() = default;

	void Blit(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView** shaderResourceView_,
		uint32_t startSlot, uint32_t numViews, ID3D11PixelShader* replacedPixelShader = nullptr);

private:
	Microsoft::WRL::ComPtr <ID3D11VertexShader>	embeddedVertexShader_;
	Microsoft::WRL::ComPtr <ID3D11PixelShader>	embeddedPixelShader_;

};
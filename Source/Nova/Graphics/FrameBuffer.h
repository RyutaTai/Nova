#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <cstdint>

class FrameBuffer
{
public:
	FrameBuffer(ID3D11Device* device, uint32_t width, uint32_t height, bool hasDepthstencil = true/*BLOOM*/);
	virtual ~FrameBuffer() = default;

	void Clear(ID3D11DeviceContext* deviceContext,
		float r = 0, float g = 0, float b = 0, float a = 1, float depth = 1);
	void Activate  (ID3D11DeviceContext* deviceContext);
	void Deactivate(ID3D11DeviceContext* deviceContext);

public:
	Microsoft::WRL::ComPtr <ID3D11RenderTargetView>		renderTargetView_;
	Microsoft::WRL::ComPtr <ID3D11DepthStencilView>		depthStencilView_;
	Microsoft::WRL::ComPtr <ID3D11ShaderResourceView>	shaderResourceViews_[2];
	D3D11_VIEWPORT										viewport_;

private:
	UINT viewportCount_{ D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE };
	D3D11_VIEWPORT cachedViewports_[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
	Microsoft::WRL::ComPtr <ID3D11RenderTargetView>		cachedRenderTargetView_;
	Microsoft::WRL::ComPtr <ID3D11DepthStencilView>		cachedDepthStencilView_;

};
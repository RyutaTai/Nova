#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <directxmath.h>

#include <cstdint>
#include <functional>

#include "../Graphics/FullScreenQuad.h"
#include "../Graphics/FrameBuffer.h"
#include "../Resources/Model.h"

class ShadowMap
{
private:
	ShadowMap();
	virtual ~ShadowMap() = default;

public:
	static ShadowMap& Instance()
	{
		static ShadowMap instance_;
		return instance_;
	}

	void Activate(float depth = 1);
	void Deactivate();

	ID3D11ShaderResourceView*	GetShaderResourceView() { return shaderResourceView_.Get(); }
	ID3D11Buffer*				GetConstantBuffer()		{ return shadowConstantBuffer_.Get(); }
	DirectX::XMFLOAT4&			GetCameraPosition()		{ return cameraPosition_; }
	DirectX::XMFLOAT4&			GetLightDirection()		{ return lightDirection_; }

	void DrawDebug();

private:
	D3D11_VIEWPORT	viewport_;
	UINT			viewportCount_{ D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE };
	D3D11_VIEWPORT	cachedViewports_[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
	Microsoft::WRL::ComPtr	<ID3D11PixelShader>			nullPixelShader_{ NULL };
	Microsoft::WRL::ComPtr	<ID3D11ShaderResourceView>	shaderResourceView_;
	Microsoft::WRL::ComPtr	<ID3D11DepthStencilView>	depthStencilView_;
	Microsoft::WRL::ComPtr	<ID3D11RenderTargetView>	cachedRenderTargetView_;
	Microsoft::WRL::ComPtr	<ID3D11DepthStencilView>	cachedDepthStencilView_;
	//ConstantBuffer
	Microsoft::WRL::ComPtr	<ID3D11Buffer>				shadowConstantBuffer_;

private:
	// SHADOW
	DirectX::XMFLOAT4 lightViewFocus_	{ 0, 0, 0, 1 };
	DirectX::XMFLOAT4 lightDirection_	{ -0.113f, -0.718f, -0.040f, 0.0f };
	DirectX::XMFLOAT4 cameraPosition_	{ 6.0f, 6.0f, -5.0f, 1.0f };
	float lightViewDistance_	{ 10.0f };
	float lightViewSize_		{ 12.0f };
	float lightViewNearZ_		{ 2.0f };
	float lightViewFarZ_		{ 18.0f };

};


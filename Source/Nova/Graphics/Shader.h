#pragma once

#include <d3d11.h>
#include <cstdio>
#include <wrl.h>
#include <memory>

class Shader
{
public:
	enum class SAMPLER_STATE {
		POINT,
		LINEAR,
		ANISOTROPIC,
		LINEAR_BORDER_BLACK,
		LINEAR_BORDER_WHITE,
		COMPARISON_LINEAR_BORDER_WHITE/*SHADOW*/,
		LINEAR_BORDER_OPAQUE_BLACK,
		POINT_CLAMP,
		Max,
	};

	enum class DEPTH_STENCIL_STATE
	{
		ZT_ON_ZW_ON,
		ZT_ON_ZW_OFF,
		ZT_OFF_ZW_ON,
		ZT_OFF_ZW_OFF,
		Max,
	};

	enum class BLEND_STATE
	{
		NONE,
		ALPHA,
		ADD,
		SUBTRACT,
		REV_SUBTRACT,	//	ReverseSubtract
		MULTIPLY,
		Max,
	};

	enum class RASTERIZER_STATE
	{
		SOLID,
		WIREFRAME,
		CULL_NONE,
		//CULL_BACK,
		WIREFRAME_CULL_NONE,
		Max,
	};

public:
	Shader();
	~Shader();

	//	ステート設定
	void SetSamplerState(ID3D11DeviceContext* deviceContext);
	void SetDepthStencilState(DEPTH_STENCIL_STATE depthStencilState);
	void SetBlendState(BLEND_STATE blendState);
	void SetRasterizerState(RASTERIZER_STATE rasterizerState);

	HRESULT CreateVsFromCso(ID3D11Device* device, const char* csoName, ID3D11VertexShader** vertexShader, ID3D11InputLayout** inputLayout,
		D3D11_INPUT_ELEMENT_DESC* inputElementDesc, UINT numElements);
	HRESULT CreatePsFromCso(ID3D11Device* device, const char* csoName, ID3D11PixelShader** pixelShader);

private:
	Microsoft::WRL::ComPtr <ID3D11SamplerState>			samplerStates_[static_cast<UINT>(SAMPLER_STATE::Max)];
	Microsoft::WRL::ComPtr <ID3D11DepthStencilState>	depthStencilStates_[static_cast<UINT>(DEPTH_STENCIL_STATE::Max)];
	Microsoft::WRL::ComPtr <ID3D11BlendState>			blendStates_[static_cast<UINT>(BLEND_STATE::Max)];
	Microsoft::WRL::ComPtr <ID3D11RasterizerState>		rasterizerStates_[static_cast<UINT>(RASTERIZER_STATE::Max)];

};
#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <directxmath.h>
#include <vector>

class Decal
{
public:
	Decal(ID3D11Device* device, const wchar_t* filename);
	virtual ~Decal() = default;

	void Add(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& normal, const float& scale);
	void Blit(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* const* depthStencilShaderResourceView);

	VOID DrawDebug();

public:
	struct Spot
	{
		DirectX::XMFLOAT3 position_ = { 0, 0, 0 };
		DirectX::XMFLOAT3 normal_	= { 1, 1, 1 };
		float scale_ = 1.0f;
	};
	std::vector<Spot> spots_;
	int spotsIndex_ = 0;	//	ImGuiでデバッグする際に使用
	int spotsIndexMax_ = 0;
	static constexpr int SpotsDataMax_ = 60;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	decalTexture_;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState>		decalDepthStencilState_;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState>		decalRasterizerStates_[2];

	//	Cuboid
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer_;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer_;

	Microsoft::WRL::ComPtr<ID3D11VertexShader>	vertexShader_;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	pixelShader_;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>	inputLayout_;

	struct Constants
	{
		DirectX::XMFLOAT4X4 world_;
		DirectX::XMFLOAT4X4 decalInverseProjection_;
	};
	Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer_;


};
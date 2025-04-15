#pragma once

#include <directxmath.h>
#include <wrl.h>
#include <d3d11.h>
#include <string>
#include <vector>

#include "../Graphics/Shader.h"

class StaticMesh
{
public:
	struct Vertex
	{
		DirectX::XMFLOAT3 position_;	//	座標
		DirectX::XMFLOAT3 normal_;		//	頂点法線
		DirectX::XMFLOAT2 texcoord_;	//	テクスチャ画像
	};
	struct Constants
	{
		DirectX::XMFLOAT4X4 world_;
		DirectX::XMFLOAT4   materialColor_;
	};
	struct Subset 
	{
		std::wstring usemtl_;
		uint32_t indexStart_{ 0 };	//	start position of index buffer
		uint32_t indexCount_{ 0 };	//	number of vertices (indices)   
	};
	std::vector<Subset> subsets_;

private:
	struct Material
	{
		std::wstring name_;
		DirectX::XMFLOAT4 Ka_{ 0.2f, 0.2f, 0.2f, 1.0f };	//	アンビエント(環境光)
		DirectX::XMFLOAT4 Kd_{ 0.8f, 0.8f, 0.8f, 1.0f };	//	ディフューズ(拡散反射光)
		DirectX::XMFLOAT4 Ks_{ 1.0f, 1.0f, 1.0f, 1.0f };	//	スペキュラ(鏡面反射光)
		std::wstring textureFilenames_[2];
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceViews_[2];
	};
	std::vector	<Material>	materials_;


public:
	StaticMesh(ID3D11Device* device,const wchar_t* objFileName,bool inverted);
	virtual ~StaticMesh() = default;

	void Render(ID3D11DeviceContext* deviceContext,
		const DirectX::XMFLOAT4X4& world, const DirectX::XMFLOAT4& materialColor);

protected:
	void CreateComBuffers(ID3D11Device* device, Vertex* vertices, size_t vertexCount,
		uint32_t* indices, size_t indexCount);

private:
	Microsoft::WRL::ComPtr <ID3D11Buffer>		vertexBuffer_;
	Microsoft::WRL::ComPtr <ID3D11Buffer>		indexBuffer_;
	Microsoft::WRL::ComPtr <ID3D11Buffer>		constantBuffer_;
	Microsoft::WRL::ComPtr <ID3D11VertexShader> vertexShader_;
	Microsoft::WRL::ComPtr <ID3D11PixelShader>	pixelShader_;
	Microsoft::WRL::ComPtr <ID3D11InputLayout>	inputLayout_;

};


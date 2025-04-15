#pragma once

#include <directxmath.h>
#include <wrl.h>
#include <d3d11.h>

#include "../Graphics/Shader.h"

class GeometricPrimitive
{
public:
	struct Vertex
	{
		DirectX::XMFLOAT3 position_;
		DirectX::XMFLOAT3 normal_;	
	};

	struct Constants
	{
		DirectX::XMFLOAT4X4 world_;
		DirectX::XMFLOAT4   materialColor_;
	};

public:
	GeometricPrimitive(ID3D11Device* device);
	virtual ~GeometricPrimitive() = default;

	void Render(ID3D11DeviceContext* deviceContext,
		const DirectX::XMFLOAT4X4& world, const DirectX::XMFLOAT4& materialColor);

protected:
	void CreateComBuffers(ID3D11Device* device, Vertex* vertices, size_t vertexCount,
		uint32_t* indices, size_t indexCount);

private:
	Microsoft::WRL::ComPtr <ID3D11Buffer>		vertexBuffer_;
	Microsoft::WRL::ComPtr <ID3D11Buffer>		indexBuffer_;
	Microsoft::WRL::ComPtr <ID3D11Buffer>		constantBuffer_;
	Microsoft::WRL::ComPtr <ID3D11VertexShader>	vertexShader_;
	Microsoft::WRL::ComPtr <ID3D11PixelShader>	pixelShader_;
	Microsoft::WRL::ComPtr <ID3D11InputLayout>	inputLayout_;

};

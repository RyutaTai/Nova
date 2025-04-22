#pragma once

#include <WICTextureLoader.h>
#include <d3d11.h>
#include <directxmath.h>
#include <vector>
#include <sstream>
#include <wrl.h>

#include "Sprite.h"
#include "Texture.h"
#include "../Graphics/Shader.h"
#include "../Others/Misc.h"

class SpriteBatch
{
private:
	struct Vertex
	{
		DirectX::XMFLOAT3 position_;
		DirectX::XMFLOAT4 color_;
		DirectX::XMFLOAT2 texcoord_;
	};
	std::vector<Vertex> vertices_;
	const size_t maxVertices_;

public:
	SpriteBatch(ID3D11Device* device, const wchar_t* filename,size_t maxSprites);
	~SpriteBatch();

	void Render(ID3D11DeviceContext* deviceContext,
		float dx, float dy, 
		float dw, float dh, 
		float r, float g, float b, float a,
		float angle/*degree*/
	);
	void Render(ID3D11DeviceContext* deviceContext,
		float dx, float dy, float dw, float dh,
		float r, float g, float b, float a,
		float angle/*degree*/,
		float sx, float sy, float sw, float sh
	);

	void Begin(ID3D11DeviceContext* deviceContext);
	void End  (ID3D11DeviceContext* deviceContext);

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer>			 vertexBuffer_;
	Microsoft::WRL::ComPtr <ID3D11VertexShader>		 vertexShader_;
	Microsoft::WRL::ComPtr <ID3D11PixelShader>		 pixelShader_;
	Microsoft::WRL::ComPtr <ID3D11InputLayout>		 inputLayout_;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView_;
	D3D11_TEXTURE2D_DESC							 texture2dDesc_;

};

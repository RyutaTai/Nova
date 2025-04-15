#include "GeometricPrimitive.h"

#include "../Graphics/Graphics.h"
#include "../Others/Misc.h"

//	コンストラクタ
GeometricPrimitive::GeometricPrimitive(ID3D11Device* device)
{
	//	サイズが1.0の正立方体データ
	//	正立方体のコントロールポイント数は8個
	//	１つのコントロールポイントの位置には法線の向きが違う頂点が３個あるため頂点情報の総数は8x3=24個、
	//	頂点情報配列(vertices)にすべて頂点の位置・法線情報を格納する。
	Vertex vertices[24]{
		//	手前の面
		{ { -1.0f, +1.0f, -1 },{ 0, 0, -1 }},	//	0
		{ { +1.0f, +1.0f, -1 },{ 0, 0, -1 }},	
		{ { -1.0f, -1.0f, -1 },{ 0, 0, -1 }},	
		{ { +1.0f, -1.0f, -1 },{ 0, 0, -1 }},	
			
		//	奥の面;
		{ { -1.0f, +1.0f, 1 },{ 0, 0, 1 }},		//	4
		{ { +1.0f, +1.0f, 1 },{ 0, 0, 1 }},	
		{ { -1.0f, -1.0f, 1 },{ 0, 0, 1 }},	
		{ { +1.0f, -1.0f, 1 },{ 0, 0, 1 }},	

		//	左の面
		{ { -1.0f, +1.0f,  1 },{-1, 0, 0 }},	//	8
		{ { -1.0f, +1.0f, -1 },{-1, 0, 0 }},
		{ { -1.0f, -1.0f,  1 },{-1, 0, 0 }},
		{ { -1.0f, -1.0f, -1 },{-1, 0, 0 }},

		//	右面
		{ { +1.0f, +1.0f, -1 },{+1, 0, 0 }},	//	12
		{ { +1.0f, +1.0f,  1 },{+1, 0, 0 }},
		{ { +1.0f, -1.0f, -1 },{+1, 0, 0 }},
		{ { +1.0f, -1.0f,  1 },{+1, 0, 0 }},

		//	上の面
		{ { -1.0f, +1.0f,  1 },{ 0, 1, 0 }},	//	16
		{ { +1.0f, +1.0f,  1 },{ 0, 1, 0 }},
		{ { -1.0f, +1.0f, -1 },{ 0, 1, 0 }},	
		{ { +1.0f, +1.0f, -1 },{ 0, 1, 0 }},	

		//	下の面
		{ { -1.0f, -1.0f,  1 },{ 0, -1, 0 }},	//	20
		{ { +1.0f, -1.0f,  1 },{ 0, -1, 0 }},
		{ { -1.0f, -1.0f, -1 },{ 0, -1, 0 }},	
		{ { +1.0f, -1.0f, -1 },{ 0, -1, 0 }},	

	};

	//	正立方体
	//	時計回りが表面になるように格納する。
	uint32_t indices[36]{
		//	verticesの要素番号
		//	奥の面
		5,4,6,
		5,6,7,

		//	右の面
		12,13,14,
		13,15,14,

		//	左の面
		8,9,10,
		9,11,10,
		
		//	下の面
		21,20,23,
		23,20,22,
		
		//	上の面
		16,17,18,
		17,19,18,

		//	手前の面
		0,1,2,
		1,3,2,
	};

	CreateComBuffers(device, vertices, 24, indices, 36);

	HRESULT hr{ S_OK };

	D3D11_INPUT_ELEMENT_DESC inputElementDesc[]
	{
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,
		D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,
		D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
	};
	Graphics::Instance().GetShader()->CreateVsFromCso(device, "./Resources/Shader/GeometricPrimitiveVs.cso", vertexShader_.GetAddressOf(),
		inputLayout_.GetAddressOf(), inputElementDesc, ARRAYSIZE(inputElementDesc));
	Graphics::Instance().GetShader()->CreatePsFromCso(device, "./Resources/Shader/GeometricPrimitivePs.cso", pixelShader_.GetAddressOf());

	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.ByteWidth = sizeof(Constants);
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	hr = device->CreateBuffer(&bufferDesc, nullptr, constantBuffer_.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

}

void GeometricPrimitive::CreateComBuffers(ID3D11Device* device, Vertex* vertices, size_t vertexCount,
	uint32_t* indices, size_t indexCount)
{
	HRESULT hr{ S_OK };

	D3D11_BUFFER_DESC bufferDesc{};
	D3D11_SUBRESOURCE_DATA subresourceData{};
	bufferDesc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * vertexCount);
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;
	subresourceData.pSysMem = vertices;
	subresourceData.SysMemPitch = 0;
	subresourceData.SysMemSlicePitch = 0;
	hr = device->CreateBuffer(&bufferDesc, &subresourceData, vertexBuffer_.ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	bufferDesc.ByteWidth = static_cast<UINT>(sizeof(uint32_t) * indexCount);
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	subresourceData.pSysMem = indices;
	hr = device->CreateBuffer(&bufferDesc, &subresourceData, indexBuffer_.ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
}

//	描画処理
void GeometricPrimitive::Render(ID3D11DeviceContext* deviceContext,
	const DirectX::XMFLOAT4X4& world, const DirectX::XMFLOAT4& materialColor)
{
	uint32_t stride{ sizeof(Vertex) };
	uint32_t offset{ 0 };
	deviceContext->IASetVertexBuffers(0, 1, vertexBuffer_.GetAddressOf(), &stride, &offset);
	deviceContext->IASetIndexBuffer(indexBuffer_.Get(), DXGI_FORMAT_R32_UINT, 0);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->IASetInputLayout(inputLayout_.Get());

	deviceContext->VSSetShader(vertexShader_.Get(), nullptr, 0);
	deviceContext->PSSetShader(pixelShader_.Get(), nullptr, 0);

	Constants data{ world,materialColor };
	deviceContext->UpdateSubresource(constantBuffer_.Get(), 0, 0, &data, 0, 0);
	deviceContext->VSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());

	D3D11_BUFFER_DESC bufferDesc{};
	indexBuffer_->GetDesc(&bufferDesc);
	deviceContext->DrawIndexed(bufferDesc.ByteWidth / sizeof(uint32_t), 0, 0);
}
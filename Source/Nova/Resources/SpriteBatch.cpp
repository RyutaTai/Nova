#include "SpriteBatch.h"

#include "../Graphics/Graphics.h"

//	コンストラクタ
SpriteBatch::SpriteBatch(ID3D11Device* device, const wchar_t* fileName, size_t maxSprites)
	:maxVertices_(maxSprites * 6)
{
	HRESULT hr{ S_OK };

	std::unique_ptr<Vertex[]>vertices{ std::make_unique<Vertex[]>(maxVertices_) };

	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.ByteWidth = sizeof(Vertex) * maxVertices_;
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA subresourceData{};
	subresourceData.pSysMem = vertices.get();
	subresourceData.SysMemPitch = 0;
	subresourceData.SysMemSlicePitch = 0;
	hr = device->CreateBuffer(&bufferDesc, &subresourceData, vertexBuffer_.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	D3D11_INPUT_ELEMENT_DESC inputElementDesc[]
	{
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,
		D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,
		D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0} ,
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
		D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 } ,
	};

	//	シェーダー読み込み
	Graphics::Instance().GetShader()->CreateVsFromCso(device, "./Resources/Shader/SpriteVs.cso", vertexShader_.GetAddressOf(), inputLayout_.GetAddressOf(), inputElementDesc, _countof(inputElementDesc));
	Graphics::Instance().GetShader()->CreatePsFromCso(device, "./Resources/Shader/SpritePs.cso", pixelShader_.GetAddressOf());

	//	テクスチャ読み込み
	// テクスチャ読み込み
	LoadTextureFromFile(device, fileName, shaderResourceView_.GetAddressOf(), &texture2dDesc_);

}

//	デストラクタ
SpriteBatch::~SpriteBatch()
{

}

//	描画処理
void SpriteBatch::Render(ID3D11DeviceContext* deviceContext, float dx, float dy, float dw, float dh,
	float r, float g, float b, float a,
	float angle)
{
	D3D11_VIEWPORT viewport{};
	UINT numViewports{ 1 };
	deviceContext->RSGetViewports(&numViewports, &viewport);

	//left-top
	float x0{ dx };
	float y0{ dy };
	//right-top
	float x1{ dx + dw };
	float y1{ dy };
	//left-bottom
	float x2{ dx };
	float y2{ dy + dh };
	//right-bottom
	float x3{ dx + dw };
	float y3{ dy + dh };

	auto rotate = [](float& x, float& y, float cx, float cy, float angle)
	{
		x -= cx;
		y -= cy;

		float cos{ cosf(DirectX::XMConvertToRadians(angle)) };
		float sin{ sinf(DirectX::XMConvertToRadians(angle)) };
		float tx{ x }, ty{ y };
		x = cos * tx + -sin * ty;
		y = sin * tx + cos * ty;

		x += cx;
		y += cy;
	};

#if 1	//	回転の中心を矩形の中心にする場合
	float cx = dx + dw * 0.5f;
	float cy = dy + dh * 0.5f;

#else	//	回転の中心を左上にする場合
	//float cx = dx;
	//float cy = dy;
#endif

	rotate(x0, y0, cx, cy, angle);
	rotate(x1, y1, cx, cy, angle);
	rotate(x2, y2, cx, cy, angle);
	rotate(x3, y3, cx, cy, angle);

	//	screen space to NDC
	x0 = 2.0f * x0 / viewport.Width - 1.0f;
	y0 = 1.0f - 2.0f * y0 / viewport.Height;
	x1 = 2.0f * x1 / viewport.Width - 1.0f;
	y1 = 1.0f - 2.0f * y1 / viewport.Height;
	x2 = 2.0f * x2 / viewport.Width - 1.0f;
	y2 = 1.0f - 2.0f * y2 / viewport.Height;
	x3 = 2.0f * x3 / viewport.Width - 1.0f;
	y3 = 1.0f - 2.0f * y3 / viewport.Height;

	HRESULT hr{ S_OK };
	D3D11_MAPPED_SUBRESOURCE mappedSubresource{};
	hr = deviceContext->Map(vertexBuffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	Vertex* vertices{ reinterpret_cast<Vertex*>(mappedSubresource.pData) };
	if (vertices != nullptr)
	{
		vertices[0].position_ = { x0,y0,0 };
		vertices[1].position_ = { x1,y1,0 };
		vertices[2].position_ = { x2,y2,0 };
		vertices[3].position_ = { x3,y3,0 };
		vertices[0].color_ = vertices[1].color_ = vertices[2].color_ = vertices[3].color_ = { r,g,b,a };

		vertices[0].texcoord_ = { 0, 0 };
		vertices[1].texcoord_ = { 1, 0 };
		vertices[2].texcoord_ = { 0, 1 };
		vertices[3].texcoord_ = { 1, 1 };

	}
	deviceContext->Unmap(vertexBuffer_.Get(), 0);

	UINT stride{ sizeof(Vertex) };
	UINT offset{ 0 };
	deviceContext->IASetVertexBuffers(0, 1, vertexBuffer_.GetAddressOf(), &stride, &offset);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	deviceContext->IASetInputLayout(inputLayout_.Get());
	deviceContext->VSSetShader(vertexShader_.Get(), nullptr, 0);
	deviceContext->PSSetShader(pixelShader_.Get(), nullptr, 0);
	deviceContext->PSSetShaderResources(0, 1, shaderResourceView_.GetAddressOf());

	deviceContext->Draw(4, 0);

}

//	描画処理
void SpriteBatch::Render(ID3D11DeviceContext* deviceContext, float dx, float dy, float dw, float dh,
	float r, float g, float b, float a, 
	float angle/*degree*/,
	float sx, float sy, float sw, float sh)
{
	D3D11_VIEWPORT viewport{};
	UINT numViewports{ 1 };
	deviceContext->RSGetViewports(&numViewports, &viewport);
	//left-top
	float x0{ dx };
	float y0{ dy };
	//right-top
	float x1{ dx + dw };
	float y1{ dy };
	//left-bottom
	float x2{ dx };
	float y2{ dy + dh };
	//right-bottom
	float x3{ dx + dw };
	float y3{ dy + dh };

	auto rotate = [](float& x, float& y, float cx, float cy, float angle)
	{
		x -= cx;
		y -= cy;

		float cos{ cosf(DirectX::XMConvertToRadians(angle)) };
		float sin{ sinf(DirectX::XMConvertToRadians(angle)) };
		float tx{ x }, ty{ y };
		x = cos * tx + -sin * ty;
		y = sin * tx + cos * ty;

		x += cx;
		y += cy;
	};

#if 1	//	回転の中心を矩形の中心にする場合
	float cx = dx + dw * 0.5f;
	float cy = dy + dh * 0.5f;

#else	//	回転の中心を左上にする場合
	 //float cx = dx;
	 //float cy = dy;
#endif

	rotate(x0, y0, cx, cy, angle);
	rotate(x1, y1, cx, cy, angle);
	rotate(x2, y2, cx, cy, angle);
	rotate(x3, y3, cx, cy, angle);

	//	screen space to NDC
	x0 = 2.0f * x0 / viewport.Width - 1.0f;
	y0 = 1.0f - 2.0f * y0 / viewport.Height;
	x1 = 2.0f * x1 / viewport.Width - 1.0f;
	y1 = 1.0f - 2.0f * y1 / viewport.Height;
	x2 = 2.0f * x2 / viewport.Width - 1.0f;
	y2 = 1.0f - 2.0f * y2 / viewport.Height;
	x3 = 2.0f * x3 / viewport.Width - 1.0f;
	y3 = 1.0f - 2.0f * y3 / viewport.Height;

	float u0{ sx / texture2dDesc_.Width };
	float v0{ sy / texture2dDesc_.Height };
	float u1{ (sx + sw) / texture2dDesc_.Width };
	float v1{ (sy + sh) / texture2dDesc_.Height };

	vertices_.push_back({ { x0,y0,0 }, { r,g,b,a }, { u0,v0 } });
	vertices_.push_back({ { x1,y1,0 }, { r,g,b,a }, { u1,v0 } });
	vertices_.push_back({ { x2,y2,0 }, { r,g,b,a }, { u0,v1 } });
	vertices_.push_back({ { x2,y2,0 }, { r,g,b,a }, { u0,v1 } });
	vertices_.push_back({ { x1,y1,0 }, { r,g,b,a }, { u1,v0 } });
	vertices_.push_back({ { x3,y3,0 }, { r,g,b,a }, { u1,v1 } });

}

void SpriteBatch::Begin(ID3D11DeviceContext* deviceContext)
{
	vertices_.clear();
	deviceContext->VSSetShader(vertexShader_.Get(), nullptr, 0);
	deviceContext->PSSetShader(pixelShader_.Get(), nullptr, 0);
	deviceContext->PSSetShaderResources(0, 1, shaderResourceView_.GetAddressOf());
}

void SpriteBatch::End(ID3D11DeviceContext* deviceContext)
{
	HRESULT hr{ S_OK };
	D3D11_MAPPED_SUBRESOURCE mappedSubresource{};
	hr = deviceContext->Map(vertexBuffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	size_t vertexCount = vertices_.size();
	_ASSERT_EXPR(maxVertices_ >= vertexCount, L"Buffer overflow");
	Vertex* data{ reinterpret_cast<Vertex*>(mappedSubresource.pData) };

	if (data != nullptr)
	{
		const Vertex* p = vertices_.data();
		memcpy_s(data, maxVertices_ * sizeof(Vertex), p, vertexCount * sizeof(Vertex));
	}
	deviceContext->Unmap(vertexBuffer_.Get(), 0);

	UINT stride{ sizeof(Vertex) };
	UINT offset{ 0 };
	deviceContext->IASetVertexBuffers(0, 1, vertexBuffer_.GetAddressOf(), &stride, &offset);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->IASetInputLayout(inputLayout_.Get());

	deviceContext->Draw(static_cast<UINT>(vertexCount), 0);
}


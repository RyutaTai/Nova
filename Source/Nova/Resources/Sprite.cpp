#include "sprite.h"

#include "../Graphics/Graphics.h"
#include "../../imgui/ImGuiCtrl.h"
#include <algorithm>

//	コンストラクタ
Sprite::Sprite(const wchar_t* filename)
{
	HRESULT hr = S_OK;
	ID3D11Device* device = Graphics::Instance().GetDevice();

	Vertex vertices[]
	{
		{ { -1.0, +1.0, 0 }, { 1, 1, 1, 1 }, { 0, 0 } },//左上
		{ { +1.0, +1.0, 0 }, { 1, 1, 1, 1 }, { 1, 0 } },//右上
		{ { -1.0, -1.0, 0 }, { 1, 1, 1, 1 }, { 0, 1 } },//左下
		{ { +1.0, -1.0, 0 }, { 1, 1, 1, 1 }, { 1, 1 } },//右下
	};

	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.ByteWidth = sizeof(vertices);
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA subresourceData{};
	subresourceData.pSysMem = vertices;
	subresourceData.SysMemPitch = 0;
	subresourceData.SysMemSlicePitch = 0;

	hr = device->CreateBuffer(&bufferDesc, &subresourceData, vertexBuffer_.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	D3D11_INPUT_ELEMENT_DESC inputElementDesc[]
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,	 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",	  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,		 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	// シェーダー読み込み
	Shader* shader = Graphics::Instance().GetShader();
	hr = shader->CreateVsFromCso(device, "./Resources/Shader/SpriteVs.cso", vertexShader_.GetAddressOf(), inputLayout_.GetAddressOf(), inputElementDesc, _countof(inputElementDesc));
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	hr = shader->CreatePsFromCso(device, "./Resources/Shader/SpritePs.cso", pixelShader_.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	// テクスチャ読み込み
	LoadTextureFromFile(device, filename, shaderResourceView_.GetAddressOf(), &texture2dDesc_);

	GetTransform()->SetSize(texture2dDesc_.Width, texture2dDesc_.Height);
	GetTransform()->SetTexSize(texture2dDesc_.Width, texture2dDesc_.Height);
}

//	デストラクタ
Sprite::~Sprite()
{

}

//	描画処理
void Sprite::Render()
{
	if (renderFlag_ == false)return;	//	描画フラグがfalseなら処理しない

#if 1
	Graphics& graphics = Graphics::Instance();

	D3D11_VIEWPORT viewport{};
	UINT numViewports{ 1 };

	graphics.GetDeviceContext()->RSGetViewports(&numViewports, &viewport);

	float width = GetTransform()->GetSizeX() * GetTransform()->GetScale().x * GetTransform()->GetScaleFactor();
	float height = GetTransform()->GetSizeY() * GetTransform()->GetScale().y * GetTransform()->GetScaleFactor();

	DirectX::XMFLOAT2 centerConvert =
	{
		width * GetTransform()->GetPivot().x,
		height * GetTransform()->GetPivot().y
	};

	//	left-top
	float x0{ GetTransform()->GetPositionX() };
	float y0{ GetTransform()->GetPositionY() };
	//	right-top
	float x1{ GetTransform()->GetPositionX() + width };
	float y1{ GetTransform()->GetPositionY() };
	//	left-bottom
	float x2{ GetTransform()->GetPositionX() };
	float y2{ GetTransform()->GetPositionY() + height };
	//	right-bottom
	float x3{ GetTransform()->GetPositionX() + width };
	float y3{ GetTransform()->GetPositionY() + height };

	x0 -= centerConvert.x; x1 -= centerConvert.x; x2 -= centerConvert.x; x3 -= centerConvert.x;
	y0 -= centerConvert.y; y1 -= centerConvert.y; y2 -= centerConvert.y; y3 -= centerConvert.y;

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

#if 1	//	回転の中心をCenterする場合
	float cx = x0 + centerConvert.x;
	float cy = y0 + centerConvert.y;

#else	//	回転の中心を左上にする場合
	float cx = GetTransform()->GetPositionX();
	float cy = GetTransform()->GetPositionY();
#endif

	rotate(x0, y0, cx, cy, GetTransform()->GetAngle());
	rotate(x1, y1, cx, cy, GetTransform()->GetAngle());
	rotate(x2, y2, cx, cy, GetTransform()->GetAngle());
	rotate(x3, y3, cx, cy, GetTransform()->GetAngle());

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
	hr = graphics.GetDeviceContext()->Map(vertexBuffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	Vertex* vertices{ reinterpret_cast<Vertex*>(mappedSubresource.pData) };
	if (vertices != nullptr)
	{
		vertices[0].position_ = { x0,y0,0 };
		vertices[1].position_ = { x1,y1,0 };
		vertices[2].position_ = { x2,y2,0 };
		vertices[3].position_ = { x3,y3,0 };
		vertices[0].color_ = vertices[1].color_ = vertices[2].color_ = vertices[3].color_ = GetTransform()->GetColor();

		float sx = GetTransform()->GetTexPosX();
		float sy = GetTransform()->GetTexPosY();
		float sw = GetTransform()->GetTexSizeX();
		float sh = GetTransform()->GetTexSizeY();

		vertices[0].texcoord_ = { sx / texture2dDesc_.Width,		sy / texture2dDesc_.Height };
		vertices[1].texcoord_ = { (sx + sw) / texture2dDesc_.Width, sy / texture2dDesc_.Height };
		vertices[2].texcoord_ = { sx / texture2dDesc_.Width,		(sy + sh) / texture2dDesc_.Height };
		vertices[3].texcoord_ = { (sx + sw) / texture2dDesc_.Width, (sy + sh) / texture2dDesc_.Height };

	}
	graphics.GetDeviceContext()->Unmap(vertexBuffer_.Get(), 0);

	UINT stride{ sizeof(Vertex) };
	UINT offset{ 0 };

	graphics.GetDeviceContext()->IASetVertexBuffers(0, 1, vertexBuffer_.GetAddressOf(), &stride, &offset);
	graphics.GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	graphics.GetDeviceContext()->IASetInputLayout(inputLayout_.Get());
	graphics.GetDeviceContext()->VSSetShader(vertexShader_.Get(), nullptr, 0);
	graphics.GetDeviceContext()->PSSetShader(pixelShader_.Get(), nullptr, 0);

	graphics.GetDeviceContext()->PSSetShaderResources(0, 1, shaderResourceView_.GetAddressOf());

	// 各種ステートの設定
	//Graphics::Instance().GetShader()->SetDepthStencilState(Shader::DEPTH_STENCIL_STATE::ZT_ON_ZW_ON);
	//Graphics::Instance().GetShader()->SetBlendState(Shader::BLEND_STATE::ALPHA);
	//Graphics::Instance().GetShader()->SetRasterizerState(Shader::RASTERIZER_STATE::CULL_NONE);

	// --- 描画 (これより下に何も書かない) ---
	graphics.GetDeviceContext()->Draw(4, 0);
#else

	//	現在設定されているビューポートからスクリーンサイズを取得する。
	D3D11_VIEWPORT viewport;
	UINT numViewports = 1;

	ID3D11DeviceContext* deviceContext = Graphics::Instance().GetDeviceContext();
	deviceContext->RSGetViewports(&numViewports, &viewport);
	float screen_width = viewport.Width;
	float screen_height = viewport.Height;

	// スプライトを構成する４頂点のスクリーン座標を計算する
	float dx = GetTransform()->GetPositionX();
	float dy = GetTransform()->GetPositionY();
	float dw = GetTransform()->GetSizeX();
	float dh = GetTransform()->GetSizeY();
	DirectX::XMFLOAT2 positions[] = {
		DirectX::XMFLOAT2(dx,      dy),			// 左上
		DirectX::XMFLOAT2(dx + dw, dy),			// 右上
		DirectX::XMFLOAT2(dx,      dy + dh),	// 左下
		DirectX::XMFLOAT2(dx + dw, dy + dh),	// 右下
	};

	// スプライトを構成する４頂点のテクスチャ座標を計算する
	float sx = GetTransform()->GetTexPosX();
	float sy = GetTransform()->GetTexPosY();
	float sw = GetTransform()->GetTexSizeX();
	float sh = GetTransform()->GetTexSizeY();
	DirectX::XMFLOAT2 texcoords[] = {
		DirectX::XMFLOAT2(sx,      sy),			// 左上
		DirectX::XMFLOAT2(sx + sw, sy),			// 右上
		DirectX::XMFLOAT2(sx,      sy + sh),	// 左下
		DirectX::XMFLOAT2(sx + sw, sy + sh),	// 右下
	};

	// スプライトの中心で回転させるために４頂点の中心位置が
	// 原点(0, 0)になるように一旦頂点を移動させる。
	float mx = dx + dw * 0.5f;
	float my = dy + dh * 0.5f;
	for (auto& p : positions)
	{
		p.x -= mx;
		p.y -= my;
	}

	// 頂点を回転させる
	const float PI = 3.141592653589793f;
	float angle = GetTransform()->GetAngle();
	float theta = angle * (PI / 180.0f);	// 角度をラジアン(θ)に変換
	float c = cosf(theta);
	float s = sinf(theta);
	for (auto& p : positions)
	{
		DirectX::XMFLOAT2 r = p;
		p.x = c * r.x + -s * r.y;
		p.y = s * r.x + c * r.y;
	}

	// 回転のために移動させた頂点を元の位置に戻す
	for (auto& p : positions)
	{
		p.x += mx;
		p.y += my;
	}

	// スクリーン座標系からNDC座標系へ変換する。
	for (auto& p : positions)
	{
		p.x = 2.0f * p.x / screen_width - 1.0f;
		p.y = 1.0f - 2.0f * p.y / screen_height;
	}

	// 頂点バッファの内容の編集を開始する。
	D3D11_MAPPED_SUBRESOURCE mappedBuffer;
	HRESULT hr = deviceContext->Map(vertexBuffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	// pDataを編集することで頂点データの内容を書き換えることができる。
	Vertex* v = static_cast<Vertex*>(mappedBuffer.pData);
	for (int i = 0; i < 4; ++i)
	{
		v[i].position_.x = positions[i].x;
		v[i].position_.y = positions[i].y;
		v[i].position_.z = 0.0f;

		DirectX::XMFLOAT4 color = GetTransform()->GetColor();
		v[i].color_.x = color.x;
		v[i].color_.y = color.y;
		v[i].color_.z = color.z;
		v[i].color_.w = color.w;

		v[i].texcoord_.x = texcoords[i].x / texture2dDesc_.Width;
		v[i].texcoord_.y = texcoords[i].y / texture2dDesc_.Height;
	}

	// 頂点バッファの内容の編集を終了する。
	deviceContext->Unmap(vertexBuffer_.Get(), 0);

{
	// パイプライン設定
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, vertexBuffer_.GetAddressOf(), &stride, &offset);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	deviceContext->IASetInputLayout(inputLayout_.Get());

	deviceContext->VSSetShader(vertexShader_.Get(), nullptr, 0);
	deviceContext->PSSetShader(pixelShader_.Get(), nullptr, 0);

	deviceContext->PSSetShaderResources(0, 1, shaderResourceView_.GetAddressOf());

	// 描画
	deviceContext->Draw(4, 0);
}
#endif

}

//	テキスト描画
void Sprite::Textout(std::string s,
	float x, float y, float w, float h, float r, float g, float b, float a)
{
	float sw = static_cast<float>(texture2dDesc_.Width / 16);
	float sh = static_cast<float>(texture2dDesc_.Height / 16);
	float carriage = 0;
	Graphics& graphics = Graphics::Instance();
	ID3D11DeviceContext* dc = Graphics::Instance().GetDeviceContext();

	for (const char c : s)
	{
		GetTransform()->SetPosition(x + carriage, y);
		GetTransform()->SetSize(w, h);
		GetTransform()->SetTexPos(sw * (c & 0x0F), sh * (c >> 4));
		GetTransform()->SetTexSize(sw, sh);
		GetTransform()->SetColor(r, g, b, a);
		Render();
		carriage += w;
	}
}

//	X方向切り取り
void Sprite::SpriteTransform::CutOutX(const float& sizeX)
{
	size_.x += sizeX;
	texSize_.x += sizeX;
}

//	Y方向切り取り
void Sprite::SpriteTransform::CutOutY(const float& sizeY)
{
	size_.x += sizeY;
	texSize_.x += sizeY;
}

//	切り取り
void Sprite::SpriteTransform::CutOut()
{
	//	x方向
	size_.x += cutSize_.x;
	texSize_.x += cutSize_.x;

	if (fabs(cutSize_.x) > 0)
		cutSize_.x = 0;

	//	y方向
	size_.y += cutSize_.y;
	texSize_.y += cutSize_.y;

	if (fabs(cutSize_.y) > 0)
		cutSize_.y = 0;

	isCut_ = false;

}

//	スケーリング
void Sprite::SpriteTransform::Scaling(const float& scaleFactor)
{
	DirectX::XMFLOAT2 size = defaultSize_;
	size.x *= scaleFactor;
	size.y *= scaleFactor;
	SetSize(size);
}

void Sprite::SpriteTransform::ScalingX(const float& scaleFactorX)
{
	DirectX::XMFLOAT2 size = defaultSize_;
	size.x *= scaleFactorX;
	SetSize(size);
}

void Sprite::SpriteTransform::ScalingY(const float& scaleFactorY)
{
	DirectX::XMFLOAT2 size = defaultSize_;
	size.y *= scaleFactorY;
	SetSize(size);
}

//	スケールリセット
void Sprite::SpriteTransform::ResetScale()
{
	SetSize(defaultSize_);
}


//	デバッグ描画
void Sprite::DrawDebug()
{
	this->GetTransform()->DrawDebug();
}

//	デバッグ描画　
void Sprite::SpriteTransform::DrawDebug()
{
#if USE_IMGUI
	//if (ImGui::TreeNode("Sprite"))
	//{
	ImGui::DragFloat2("Position", &position_.x);
	ImGui::DragFloat2("Pivot", &pivot_.x);
	ImGui::DragFloat("Size", &size_.x);
	ImGui::DragFloat2("TexPos", &texPos_.x);
	ImGui::DragFloat2("TexSize", &texSize_.x);
	ImGui::DragFloat2("DefaultSize", &defaultSize_.x);
	ImGui::DragFloat("Angle", &angle_);
	ImGui::DragFloat2("Scale", &scale_.x, 0.01f);
	ImGui::DragFloat("ScaleFactor", &scaleFactor_, 0.01f);

	ImGui::ColorEdit4("Color", &color_.x);

	ImGui::Checkbox("IsDebugSize", &isCut_);		//	画像切り抜き
	if (isCut_)
	{
		ImGui::DragFloat2("DebugSize", &cutSize_.x);
		CutOut();
	}
	//ImGui::TreePop();
//}
#endif
}


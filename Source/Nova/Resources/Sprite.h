#pragma once

#include <d3d11.h>
#include <directxmath.h>
#include <sstream>
#include <wrl.h>
#include <WICTextureLoader.h>

#include "../Others/Misc.h"
#include "../Graphics/Shader.h"
#include "../Resources/Texture.h"

class Sprite
{
public:	
	struct InitInfo
	{
		std::string psFilename_ = {};
		std::string vsFilename_ = {};
	};
	
	//	Sprite情報
	struct SpriteTransform
	{
	public:
		void DrawDebug();	//	ImGui描画

		//	----- 位置 -----
		void SetPosition(const DirectX::XMFLOAT2& pos) { position_ = pos; }
		void SetPosition(const float& x, const float& y) { position_ = { x, y }; }
		void SetPositionX(const float& x) { position_.x = x; }
		void SetPositionY(const float& y) { position_.y = y; }
		void AddPosition(const float& x, const float& y) { position_.x += x; position_.y += y; }
		void AddPositionX(const float& addX) { position_.x += addX; }
		void AddPositionY(const float& addY) { position_.y += addY; }
		DirectX::XMFLOAT2 GetPosition() { return position_; }
		float GetPositionX() { return position_.x; }
		float GetPositionY() { return position_.y; }

		//	----- 基準点 -----
		void SetPivot(const DirectX::XMFLOAT2& pivot) { pivot_ = pivot; }
		void SetPivot(const float& x, const float& y) { pivot_ = { x, y }; }
		void SetPivotX(const float& x) { pivot_.x = x; }
		void SetPivotY(const float& y) { pivot_.y = y; }
		DirectX::XMFLOAT2 GetPivot() { return pivot_; }
		float GetCenterX() { return pivot_.x; }
		float GetPivotY() { return pivot_.y; }

		//	----- スケール -----
		void SetScale(const DirectX::XMFLOAT2& scale) { scale_ = scale; }
		void SetScaleFactor(const float& scaleFactor) { scaleFactor_ = scaleFactor; }
		DirectX::XMFLOAT2 GetScale() { return scale_; }
		float GetScaleFactor() { return scaleFactor_; }
		void Scaling(const float& scaleFactor);
		void ScalingX(const float& scaleFactorX);
		void ScalingY(const float& scaleFactorY);
		void ResetScale();

		//	----- サイズ -----
		void SetSize(const DirectX::XMFLOAT2& size) { size_ = size; }
		void SetSize(const float& x, const float& y) { size_ = { x, y }; }
		void SetSizeX(const float& x) { size_.x = x; }
		void SetSizeY(const float& y) { size_.y = y; }
		DirectX::XMFLOAT2 GetSize() { return size_; }
		float GetSizeX() { return size_.x; }
		float GetSizeY() { return size_.y; }

		//	----- デフォルトサイズ -----
		void SetDefaultSize(const DirectX::XMFLOAT2& size) { defaultSize_ = size; }
		void SetDefaultSize(const float& sizeX, const float& sizeY) { defaultSize_ = { sizeX,sizeY }; }
		const DirectX::XMFLOAT2 GetDefaultSize()const { return defaultSize_; }

		//	----- 切り取り位置 -----
		void SetTexPos(const DirectX::XMFLOAT2& texPos) { texPos_ = texPos; }
		void SetTexPos(const float& x, const float& y) { texPos_ = { x, y }; }
		void SetTexPosX(const float& x) { texPos_.x = x; }
		void SetTexPosY(const float& y) { texPos_.y = y; }
		void AddTexPosX(const float& x) { texPos_.x += x; }
		void AddTexPosY(const float& y) { texPos_.y += y; }
		DirectX::XMFLOAT2 GetTexPos() { return texPos_; }
		float GetTexPosX() { return texPos_.x; }
		float GetTexPosY() { return texPos_.y; }

		//	----- 切り取りサイズ -----
		void SetTexSize(const DirectX::XMFLOAT2& texSize) { texSize_ = texSize; }
		void SetTexSize(const float& x, const float& y) { texSize_ = { x, y }; }
		void SetTexSizeX(const float& x) { texSize_.x = x; }
		void SetTexSizeY(const float& y) { texSize_.y = y; }
		DirectX::XMFLOAT2 GetTexSize() { return texSize_; }
		float GetTexSizeX() { return texSize_.x; }
		float GetTexSizeY() { return texSize_.y; }

		//	----- 色 -----
		void SetColor(const DirectX::XMFLOAT4& color) { color_ = color; }
		void SetColor(const float& r, const float& g, const float& b, const float& a) { color_ = { r,g,b,a }; }
		void SetColorR(const float& r) { color_.x = r; }
		void SetColorG(const float& g) { color_.y = g; }
		void SetColorB(const float& b) { color_.z = b; }
		void SetColorA(const float& a) { color_.w = a; }
		DirectX::XMFLOAT4 GetColor() { return color_; }

		//	----- 角度 -----
		void SetAngle(const float& angle) { angle_ = angle; }
		float GetAngle() { return angle_; }

	private:
		DirectX::XMFLOAT2	position_ = {};			//	位置 
		DirectX::XMFLOAT2	pivot_ = {};			//	基準点
		DirectX::XMFLOAT2	size_ = {};				//	画像サイズ
		DirectX::XMFLOAT2	texPos_ = {};			//	切り取り開始位置
		DirectX::XMFLOAT2	texSize_ = {};			//	切り取りサイズ
		DirectX::XMFLOAT2	scale_ = { 1.0f,1.0f };	//	スケール
		float				scaleFactor_ = 1.0f;	//	スケール
		DirectX::XMFLOAT4	color_ = { 1,1,1,1 };	//	描画色 
		float				angle_ = 0.0f;			//	回転角度

	private:
		DirectX::XMFLOAT2 defaultSize_ = {};		//	デフォルトサイズ(スケール1.0fのサイズ)を保持する
	};

private:
	struct Vertex
	{
		DirectX::XMFLOAT3 position_ = {};
		DirectX::XMFLOAT4 color_ = { 0.0f,0.0f,0.0f,1.0f };
		DirectX::XMFLOAT2 texcoord_ = {};
	};

public:
	Sprite(const wchar_t* filename, const InitInfo& initInfo = {});
	~Sprite() = default;

	void Render();
	void Render(uint32_t slot,ID3D11Buffer** ppConstantBuffer);
	void DrawDebug();

	//	テキスト描画
	void Textout(std::string s,
		float x, float y, float w, float h, float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f);

	//	名前
	void SetName(const std::string& name)		{ name_ = name; }
	const std::string GetName()const { return name_; }
	
	//	描画フラグ
	void SetRenderFlag(const bool& renderFlag)	{ renderFlag_ = renderFlag; }
	const bool GetRenderFlag()const { return renderFlag_; }

	//	スプライトトランスフォーム取得
	SpriteTransform*	GetTransform()	{ return &transform_; }

private:
	Microsoft::WRL::ComPtr <ID3D11Buffer>				vertexBuffer_;
	Microsoft::WRL::ComPtr <ID3D11VertexShader>			vertexShader_;
	Microsoft::WRL::ComPtr <ID3D11PixelShader>			pixelShader_;
	Microsoft::WRL::ComPtr <ID3D11InputLayout>			inputLayout_;
	Microsoft::WRL::ComPtr <ID3D11ShaderResourceView>	shaderResourceView_;
	D3D11_TEXTURE2D_DESC								texture2dDesc_;

	SpriteTransform										transform_ = {};
	std::string name_ = "";		//	スプライト名
	bool renderFlag_ = true;	//	描画フラグ

};

#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <string>

#include "../../imgui/imgui.h"
#include "../../imgui/imgui_internal.h"

class Transform
{
public:
	enum class CoordinateSystem
	{
		cRightYup,
		cLeftYup,
		cRightZup,
		cLeftZup,
		cNone,
	};

	Transform() :
		position_(DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f))
		, scale_(DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f))
		, rotation_(DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f))
		, color_(DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f))
		, scaleFactor_(1.0f)
		, coordinateSystem_(static_cast<int>(CoordinateSystem::cRightYup))
	{}
    ~Transform() {}

	//---------- ImGui用関数 ----------
	void DrawDebug();
	void Reset();
	//---------- ImGui用関数 ----------

public:// Setter,Getter, etc...
    [[nodiscard]] DirectX::XMMATRIX CalcWorld();							//	ワールド行列算出
	[[nodiscard]] DirectX::XMMATRIX CalcWorldMatrix(float scaleFactor);		//	座標系を掛けたワールド行列算出
	[[nodiscard]] DirectX::XMFLOAT3 CalcForward() const;					//	前方向ベクトル算出
	[[nodiscard]] DirectX::XMFLOAT3 CalcUp() const;							//	上方向ベクトル算出
	[[nodiscard]] DirectX::XMFLOAT3 CalcRight() const;						//	右方向ベクトル算出

	void SetWorld(DirectX::XMMATRIX world);

	//---------- 位置 ( Position ) ----------
	void SetPosition(const DirectX::XMFLOAT3& pos) { position_ = pos; }
	void SetPositionX(const float& posX) { position_.x = posX; }
	void SetPositionY(const float& posY) { position_.y = posY; }
	void SetPositionZ(const float& posZ) { position_.z = posZ; }

	[[nodiscard]] const DirectX::XMFLOAT3& GetPosition() const { return position_; }
	[[nodiscard]] const float GetPositionX() const { return position_.x; }
	[[nodiscard]] const float GetPositionY() const { return position_.y; }
	[[nodiscard]] const float GetPositionZ() const { return position_.z; }

	void AddPosition(const DirectX::XMFLOAT3& pos)
	{
		position_.x += pos.x;
		position_.y += pos.y;
		position_.z += pos.z;
	}
	void AddPositionX(const float posX) { position_.x += posX; }
	void AddPositionY(const float posY) { position_.y += posY; }
	void AddPositionZ(const float posZ) { position_.z += posZ; }

	//---------- 大きさ ( Scale ) ----------
	void SetScale(const DirectX::XMFLOAT3& scl) { scale_ = scl; }

	// ScaleFactor
	[[nodiscard]] float GetScaleFactor() const { return scaleFactor_; }
	void SetScaleFactor(float f) { scaleFactor_ = f; }

	[[nodiscard]] DirectX::XMFLOAT3 GetScale() const { return scale_; }

	//---------- 回転 ( Rotation ) ----------
	[[nodiscard]] DirectX::XMFLOAT4 GetRotation() const { return rotation_; }
	[[nodiscard]] const float GetRotationX() const { return rotation_.x; }
	[[nodiscard]] const float GetRotationY() const { return rotation_.y; }
	[[nodiscard]] const float GetRotationZ() const { return rotation_.z; }

	void SetRotation(const DirectX::XMFLOAT4& rot) { rotation_ = rot; }
	void SetRotationX(const float rotX) { rotation_.x = rotX; }
	void SetRotationY(const float rotY) { rotation_.y = rotY; }
	void SetRotationZ(const float rotZ) { rotation_.z = rotZ; }

	void AddRotationX(const float rotX) { rotation_.x += rotX; }
	void AddRotationY(const float rotY) { rotation_.y += rotY; }
	void AddRotationZ(const float rotZ) { rotation_.z += rotZ; }

	//---------- 色 ( Color ) ----------
	void SetColor(const DirectX::XMFLOAT4& color) { color_ = color; }
	void SetColorR(const float colorR) { color_.x = colorR; }
	void SetColorG(const float colorG) { color_.y = colorG; }
	void SetColorB(const float colorB) { color_.z = colorB; }
	void SetColorA(const float colorA) { color_.w = colorA; }

	[[nodiscard]] DirectX::XMFLOAT4 GetColor() const { return color_; }
	[[nodiscard]] const float GetColorR() const { return color_.x; }
	[[nodiscard]] const float GetColorG() const { return color_.y; }
	[[nodiscard]] const float GetColorB() const { return color_.z; }
	[[nodiscard]] const float GetColorA() const { return color_.w; }

	//---------- CoordinateSystem ----------
	void SetCoordinateSystem(const CoordinateSystem& coordinateSystem) { coordinateSystem_ = static_cast<int>(coordinateSystem); }

	[[nodiscard]] const DirectX::XMFLOAT4X4 GetCoordinateSystemTransform(CoordinateSystem coordinateSystem)const
	{
		return CoordinateSystemTransforms_[static_cast<int>(coordinateSystem)];
	}

private:
	DirectX::XMFLOAT3 position_ = { 0,0,0 };
	DirectX::XMFLOAT3 scale_	= { 1,1,1 };
	DirectX::XMFLOAT4 rotation_ = { 0,0,0,1 };
	DirectX::XMFLOAT4 color_	= {1,1,1,1};

	float scaleFactor_ = 1.0f;

	int coordinateSystem_;
	const DirectX::XMFLOAT4X4 CoordinateSystemTransforms_[static_cast<int>(CoordinateSystem::cNone)]
	{
		{
			-1, 0, 0, 0,
			 0, 1, 0, 0,
			 0, 0, 1, 0,
			 0, 0, 0, 1,
		},

		{
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1
		},

		{
			1, 0, 0, 0,
			0, 0, 1, 0,
			0, 1, 0, 0,
			0, 0, 0, 1
		},
		{
			1, 0, 0, 0,
			0, 0, 1, 0,
			0, 1, 0, 0,
			0, 0, 0, 1
		}
	};

	const std::string coordinateSystemName_[static_cast<int>(CoordinateSystem::cNone)]
	{
		"Right Hand, Y up",
		"Left Hand, Y up",
		"Right Hand, Z up",
		"Left Hand, Z up"
	};
};


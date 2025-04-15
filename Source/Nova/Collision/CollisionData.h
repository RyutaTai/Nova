#pragma once

#include <DirectXMath.h>
#include <string>

#include "../../Nova/Others/MathHelper.h"

//	球判定用データ
struct CollisionSphereData
{
public:
	CollisionSphereData(const std::string& name, const float& radius,
		const DirectX::XMFLOAT3& offsetPos = {}, const DirectX::XMFLOAT4& defaultColor = { 0.0f,0.0f,0.0f,1.0f }, const DirectX::XMFLOAT4& hitColor = { 1.0f,1.0f,1.0f,1.0f })
		:name_(name),offsetPosition_(offsetPos),radius_(radius),
		defaultColor_(defaultColor), currentColor_(defaultColor), hitColor_(hitColor)
	{}
	CollisionSphereData() = default;

	//	ImGUi描画
	void DrawDebug();

	//	名前
	void SetName(const std::string& name) { name_ = name; }
	const std::string GetName()const { return name_; }

	//	基準にするジョイント位置
	void SetJointPosition(const DirectX::XMFLOAT3& jointPos) { jointPosition_ = jointPos; }
	const DirectX::XMFLOAT3 GetJointPosition()const { return jointPosition_; }

	//	ジョイント位置からのオフセット値
	void SetOffSetPosition(const DirectX::XMFLOAT3& offsetPos) { offsetPosition_ = offsetPos; }
	const DirectX::XMFLOAT3 GetOffsetPosition() const{ return offsetPosition_; }

	//	半径
	void SetRadius(const float& radius) { radius_ = radius; }
	const float GetRadius()const { return radius_; }

	//	現在のカラー
	void SetColor(const DirectX::XMFLOAT4& color) { currentColor_= color; }
	const DirectX::XMFLOAT4 GetColor() const { return currentColor_; }

	//	デフォルトカラー
	void SetDefaultColor(const DirectX::XMFLOAT4& color) { defaultColor_ = color; }
	const DirectX::XMFLOAT4 GetDefaultColor() const{ return defaultColor_; }

	//	ヒット時のカラー
	void SetHitColor(const DirectX::XMFLOAT4& color) { hitColor_ = color; }
	const DirectX::XMFLOAT4 GetHitColor()const { return hitColor_; }

private:
	std::string			name_ = {};									//	名前
	DirectX::XMFLOAT3	jointPosition_ = {};						//	基準にするジョイントの位置
	DirectX::XMFLOAT3	offsetPosition_ = {};						//	ジョイント位置からのオフセット値
	float				radius_ = 0.0f;								//	半径
	DirectX::XMFLOAT4	currentColor_ = { 1.0f,1.0f,1.0f,1.0f };	//	現在のカラー
	DirectX::XMFLOAT4	defaultColor_ = { 1.0f,1.0f,1.0f,1.0f };	//	デフォルトカラー
	DirectX::XMFLOAT4	hitColor_ = { 1.0f,1.0f,1.0f,1.0f };		//	ヒット時のカラー

};

//	円柱判定用データ
struct CollisionCylinderData
{
public:
	CollisionCylinderData(const std::string& name, const DirectX::XMFLOAT3& jointPos,
		const float& radius, const float& height, const DirectX::XMFLOAT3& offsetPos = {},
		const DirectX::XMFLOAT4& defaultColor = { 0.0f,0.0f,0.0f,1.0f }, const DirectX::XMFLOAT4& hitColor = { 1.0f,1.0f,1.0f,1.0f })
		:name_(name),jointPosition_(jointPos),offsetPosition_(offsetPos),radius_(radius),height_(height),
		defaultColor_(defaultColor), currentColor_(defaultColor), hitColor_(hitColor)
	{}
	CollisionCylinderData() = default;

	//	ImGUi描画
	void DrawDebug();

	//	名前
	void SetName(const std::string& name) { name_ = name; }
	const std::string GetName()const { return name_; }

	//	基準にするジョイント位置
	void SetPosition(const DirectX::XMFLOAT3& jointPos) { jointPosition_ = jointPos; }
	const DirectX::XMFLOAT3 GetPosition()const { return jointPosition_; }

	//	ジョイント位置からのオフセット値
	void SetOffSetPosition(const DirectX::XMFLOAT3& offsetPos) { offsetPosition_ = offsetPos; }
	const DirectX::XMFLOAT3 GetOffsetPosition() const { return offsetPosition_; }

	//	半径
	void SetRadius(const float& radius) { radius_ = radius; }
	const float GetRadius() const{ return radius_; }

	//	高さ
	void SetHeight(const float& height) { height_ = height; }
	const float GetHeight() const { return height_; }

	//	現在のカラー
	void SetColor(const DirectX::XMFLOAT4& color) { currentColor_ = color; }
	const DirectX::XMFLOAT4 GetColor() const { return currentColor_; }

	//	デフォルトカラー
	void SetDefaultColor(const DirectX::XMFLOAT4& color) { defaultColor_ = color; }
	const DirectX::XMFLOAT4 GetDefaultColor() const{ return defaultColor_; }

	//	ヒット時のカラー
	void SetHitColor(const DirectX::XMFLOAT4& color) { hitColor_ = color; }
	const DirectX::XMFLOAT4 GetHitColor() const{ return hitColor_; }

private:
	std::string			name_ = {};				//	名前
	DirectX::XMFLOAT3	jointPosition_ = {};	//	基準にするジョイントの位置
	DirectX::XMFLOAT3	offsetPosition_ = {};	//	ジョイント位置からのオフセット値
	float				radius_ = 0.0f;			//	半径
	float				height_ = 0.0f;			//	高さ
	DirectX::XMFLOAT4	currentColor_ = { 1.0f,1.0f,1.0f,1.0f };	//	現在のカラー
	DirectX::XMFLOAT4	defaultColor_ = { 1.0f,1.0f,1.0f,1.0f };	//	デフォルトカラー
	DirectX::XMFLOAT4	hitColor_ = { 1.0f,1.0f,1.0f,1.0f };		//	ヒット時のカラー

};

//	攻撃判定
struct AttackDetectionData
{
public:
	AttackDetectionData(const std::string& name, const float& radius, const DirectX::XMFLOAT3& offsetPos = {},
		const std::string& updateName = "",
		const DirectX::XMFLOAT4& defaultColor = { 1.0f,0.0f,0.0f,1.0f }, const DirectX::XMFLOAT4& hitColor = { 1.0f,1.0f,1.0f,1.0f })
		:collisionSphereData_({ name,radius,offsetPos,defaultColor,hitColor }),
		updateName_((updateName == "") ? name : updateName)
	{}
	AttackDetectionData() = default;

	//	ImGUi描画
	void DrawDebug();

	// ---------- 名前 ----------
	const std::string GetName() const { return collisionSphereData_.GetName(); }

	// ---------- 半径 ----------
	const float GetRadius() const { return collisionSphereData_.GetRadius(); }

	// ---------- 位置 ----------
	const DirectX::XMFLOAT3 GetOffsetPosition() const { return collisionSphereData_.GetOffsetPosition(); }
	const DirectX::XMFLOAT3 GetPosition() const { return collisionSphereData_.GetJointPosition(); }
	void SetJointPosition(const DirectX::XMFLOAT3& position) { collisionSphereData_.SetJointPosition(position); }

	// ---------- 色 ----------
	const DirectX::XMFLOAT4 GetColor() const { return collisionSphereData_.GetColor(); }

	// ---------- 有効(使用)フラグ ----------
	void SetIsActive(const bool& isActive) { isActive_ = isActive; }
	const bool GetIsActive() const { return isActive_; }

	// ---------- 更新用名前 ----------
	const std::string GetUpdateName() const { return updateName_; }

private:
	CollisionSphereData collisionSphereData_ = {};	//	球体データ
	std::string         updateName_ = {};           //	更新用の名前
	bool                isActive_ = false;			//	現在有効か

};

//	くらい判定
struct DamageDetectionData
{
public:
	DamageDetectionData(const std::string& name, const float& radius, const DirectX::XMFLOAT3& offsetPos = {}, const float& damage = 1.0f,
		const std::string& updateName = "",
		const DirectX::XMFLOAT4& defaultColor = { 0.0f,1.0f,0.0f,1.0f }, const DirectX::XMFLOAT4& hitColor = { 1.0f,1.0f,1.0f,1.0f })
		:collisionSphereData_({ name,radius,offsetPos,defaultColor,hitColor }),
		damage_(damage),
		updateName_((updateName == "") ? name : updateName)
	{}
	DamageDetectionData() = default;

	void Update(const float& elapsedTime);

	//	ImGUi描画
	void DrawDebug();

	//	---------- 名前 ----------
	void SetName(const std::string& name) { collisionSphereData_.SetName(name); }
	const std::string GetName() const { return collisionSphereData_.GetName(); }

	//	---------- 半径 ----------
	void SetRadius(const float& radius) { collisionSphereData_.SetRadius(radius); }
	const float GetRadius() const { return collisionSphereData_.GetRadius(); }

	//	---------- 位置 ----------
	void SetJointPosition(const DirectX::XMFLOAT3& position) { collisionSphereData_.SetJointPosition(position); }
	void SetPosition(const DirectX::XMFLOAT3& jointPosition) { collisionSphereData_.SetJointPosition(jointPosition + collisionSphereData_.GetOffsetPosition()); }
	const DirectX::XMFLOAT3 GetOffsetPosition() const { return collisionSphereData_.GetOffsetPosition(); }
	const DirectX::XMFLOAT3 GetPosition() const { return collisionSphereData_.GetJointPosition(); }
	//const DirectX::XMFLOAT3 GetCollisionPosition() const { return collisionSphereData_.GetPosition() + collisionSphereData_.GetOffsetPosition(); }

	//	---------- 色 ----------
	//	現在のカラー
	void SetColor(const DirectX::XMFLOAT4& color) { collisionSphereData_.SetColor(color); }
	const DirectX::XMFLOAT4 GetColor() const { return collisionSphereData_.GetColor(); }
	//	デフォルトカラー
	void SetDefaultColor(const DirectX::XMFLOAT4& color) { collisionSphereData_.SetDefaultColor(color); }
	const DirectX::XMFLOAT4 GetDefaultColor() const { return collisionSphereData_.GetDefaultColor(); }
	//	ヒット時のカラー
	void SetHitColor(const DirectX::XMFLOAT4& color) { collisionSphereData_.SetHitColor(color); }
	const DirectX::XMFLOAT4 GetHitColor() const { return collisionSphereData_.GetHitColor(); }

	// ---------- 更新用名前 ----------
	const std::string GetUpdateName() const { return updateName_; }

	//	----- 当たっているかのフラグ -----
	void SetIsHit(const bool& hit) { isHit_ = hit; }
	const bool IsHit() const { return isHit_; }

	//	----- ダメージ倍率 -----
	void SetDamage(const float& damage) { damage_ = damage; }
	const float GetDamage()const { return damage_; }

	void SetHitTimer(const float& time) { hitTimer_ = time; }
	const float GetHitTimer()const { return hitTimer_; }

private:
	CollisionSphereData collisionSphereData_ = {};	//	球体データ

	float damage_ = 1.0f;		//	ダメージ倍率
	bool  isHit_ = false;		//	当たっているか
	float hitTimer_ = 0.0f;

	std::string updateName_ = {};	//	更新用の名前

};

//	押し出し判定
struct CollisionDetectionData
{
	CollisionDetectionData(const std::string& name, const float& radius, const bool& fixedY = false,
		const DirectX::XMFLOAT3& offsetPosition = {}, const std::string& updateName = "", 
		const DirectX::XMFLOAT4& defaultColor = { 0.0f,0.0f,0.0f,1.0f, }, const DirectX::XMFLOAT4& hitColor = { 1.0f,1.0f,1.0f,1.0f })
		: collisionSphereData_(name, radius, offsetPosition, defaultColor, hitColor),
		updateName_((updateName == "") ? name : updateName),
		fixedY_(fixedY)
	{}
	CollisionDetectionData() = default;

	//	ImGUi描画
	void DrawDebug();

	//	---------- 名前 ----------
	void SetName(const std::string& name) { collisionSphereData_.SetName(name); }
	const std::string GetName() const { return collisionSphereData_.GetName(); }

	//	---------- 半径 ----------
	void SetRadius(const float& radius) { collisionSphereData_.SetRadius(radius); }
	const float GetRadius() const { return collisionSphereData_.GetRadius(); }

	//	---------- 位置 ----------
	void SetPosition(const DirectX::XMFLOAT3& position) { collisionSphereData_.SetJointPosition(position); }
	void SetJointPosition(const DirectX::XMFLOAT3& jointPosition) { collisionSphereData_.SetJointPosition(jointPosition + collisionSphereData_.GetOffsetPosition()); }
	const DirectX::XMFLOAT3 GetOffsetPosition() const { return collisionSphereData_.GetOffsetPosition(); }
	const DirectX::XMFLOAT3 GetPosition() const { return collisionSphereData_.GetJointPosition(); }
	//const DirectX::XMFLOAT3 GetCollisionPosition() const { return collisionSphereData_.GetPosition() + collisionSphereData_.GetOffsetPosition(); }

	//	---------- 色 ----------
	//	現在のカラー
	void SetColor(const DirectX::XMFLOAT4& color) { collisionSphereData_.SetColor(color); }
	const DirectX::XMFLOAT4 GetColor() const { return collisionSphereData_.GetColor(); }
	//	デフォルトカラー
	void SetDefaultColor(const DirectX::XMFLOAT4& color) { collisionSphereData_.SetDefaultColor(color); }
	const DirectX::XMFLOAT4 GetDefaultColor() const { return collisionSphereData_.GetDefaultColor(); }
	//	ヒット時のカラー
	void SetHitColor(const DirectX::XMFLOAT4& color) { collisionSphereData_.SetHitColor(color); }
	const DirectX::XMFLOAT4 GetHitColor() const { return collisionSphereData_.GetHitColor(); }

	//	---------- 有効(使用)フラグ ----------
	void SetIsActive(const bool& isActive) { isActive_ = isActive; }
	const bool GetIsActive() const { return isActive_; }

	//	---------- 更新用名前 ----------
	const std::string GetUpdateName() const { return updateName_; }

	//	---------- Yの値が固定 ----------
	const bool GetFixedY() const { return fixedY_; }

private:
	CollisionSphereData collisionSphereData_ = {};	//	球判定用データ
	bool                isActive_ = true;			//	現在有効か

	bool				fixedY_ = false;			//	Yの値を固定するかどうか

	std::string			updateName_ = {};			//	更新名
};
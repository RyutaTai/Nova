#pragma once

#include "DirectXMath.h"
#include "../Nova/Resources/GltfModel.h"
#include "../Nova/Collision/CollisionData.h"

class Character
{
public:
	Character(const std::string& filename, const std::string& rootNodeName = "root");
	virtual ~Character() {}

	virtual void Initialize() = 0;
	virtual void Update(const float& elapsedTime);
	virtual bool RayVsVertical(const float& elapsedTime) = 0;		//	ステージとの当たり判定
	virtual bool RayVsHorizontal(const float& elapsedTime) = 0;
	virtual void Render();
	virtual void DrawDebug();	//	デバッグ描画

	//	----- 移動 -----
	virtual void Move(const float& elpasedTime);
	void SetMoveVec(const DirectX::XMFLOAT3& vec) { moveVec_ = vec; }
	//	----- 旋回処理 -----
	virtual void Turn(const float& elapsedTime, float vx, float vz, float speed);
	//	----- 速さ -----
	void AddMoveSpeed(const float& addMoveSpeed, const float& elapsedTime);
	void SetMoveSpeed(const float& moveSpeed){ moveSpeed_ = moveSpeed; }
	void ResetMoveSpeed() { moveSpeed_ = defaultMoveSpeed_; }
	const float	GetMoveSpeed()const { return moveSpeed_; }
	void SetDefaultMoveSpeed(const float& moveSpeed) { defaultMoveSpeed_ = moveSpeed; }
	const float GetDefaultMoveSpeed()const { return defaultMoveSpeed_; }

	//	----- 速度 -----
	void UpdateVelocity(const float& elapsedTime);
	void AddVelocity(const DirectX::XMFLOAT3& addVelocity,const float& elapsedTime);
	void AddVelocityY(const float& addVelocityY, const float& elapsedTime);
	void AddVelocityXZ(const float& addVelocityX, const float& addVelocityZ, const float& elapsedTime);
	void MultiplyVelocityXZ(const float& multiplyVelocity, const float& elapsedTime);
	void SetVelocity(const DirectX::XMFLOAT3& velocity)			{ velocity_ = velocity; }
	const DirectX::XMFLOAT3 GetVelocity()const { return velocity_; }
	//	----- 加速度 -----
	void AddAcceleration(const DirectX::XMFLOAT3& addAcceleration, const float& elapsedTime);
	void AddAccelerationY(const float& addAccelerationY, const float& elapsedTime);
	void AddAccelerationXZ(const float& addAccelerationX, const float& addAccelerationZ, const float& elapsedTime);
	void SetAcceleration(const DirectX::XMFLOAT3& acceleration) { acceleration_ = acceleration; }
	const DirectX::XMFLOAT3 GetAcceleration()const { return acceleration_; }
	
	//	----- 吹っ飛ばし -----
	void UpdateForce(const float& elapsedTime);
	void AddForce(const DirectX::XMFLOAT3& direction, const float& power, const float& decelerationForce);

	//	----- アニメーション -----
	void		PlayAnimation(const int& index, const bool& loop = false, const float& blendTime = 1.0f, const float& animSpeed = 1.0f, const float& startFrame = 0.0f, const float& endFrame = 0.0f);
	void		UpdateAnimation(const float& elapsedTime);
	void		AppendAnimation(const std::string& filename);
	bool		IsPlayAnimation()const;
	bool		IsBlendAnimation()const { return gltfModelResource_->IsBlendAnimation(); }
	void		SetAnimationSpeed(const float& animationSpeed)			{ gltfModelResource_->SetAnimationSpeed(animationSpeed); }
	const float	GetCurrentAnimationSeconds()const{ return gltfModelResource_->GetCurrentAnimationSeconds(); }		//	現在再生中ののアニメーション再生時間取得
	int			GetCurrentAnimNum()			const{ return gltfModelResource_->GetCurrentAnimNum(); }				//	現在再生中のアニメーション番号取得
	const float GetAnimationDuration(const int& animIndex)const { return gltfModelResource_->GetAnimationDuration(animIndex); }

	//	-----	ピクセルシェーダー -----
	void SetPixelShader(const char* csoName);
	
	//	----- HP -----
	void			SubtractHp(const int& hp);
	void			SetHp(const int& hp){ hp_ = hp; }
	const int		GetHp()const { return hp_; }

	//	----- 死亡フラグ -----
	void			SetIsDead(const bool& isDead) { isDead_ = isDead; }
	const bool		IsDead()const { return isDead_; }

	//	----- 無敵処理 -----
	void		SetIsInvincible(const bool& isInvincible)		{ isInvincible_ = isInvincible; }
	const bool	IsInvincible()const								{ return isInvincible_; }
	void		SetInvincibleTimer(const float& invincibleTimer){ invincibleTimer_ = invincibleTimer; }
	const float	GetInvincibleTimer()const						{ return invincibleTimer_; }

	//	----- ジョイントポジション -----
	DirectX::XMFLOAT3 GetJointPosition(const std::string& nodeName, const DirectX::XMFLOAT3& offsetPos = {});											//	ジョイントポジション取得
	DirectX::XMFLOAT3 GetJointPosition(const size_t& nodeIndex, const DirectX::XMFLOAT3& offsetPos = {});

	//	----- ルートモーション -----
	const int GetNodeIndex(const std::string& nodeName) { return gltfModelResource_->GetNodeIndex(nodeName); }
	void RootMotion() { gltfModelResource_->RootMotion(GetTransform()->GetScaleFactor()); }
	void SetRootJointIndex(const int& index) { gltfModelResource_->SetRootJointIndex(index); }
	void SetUseRootMotion(const bool& useRootMotion) { gltfModelResource_->SetUseRootMotion(useRootMotion); }
	void SetRootMotionSpeed(const float& rootMotionSpeed) { gltfModelResource_->SetRootMotionSpeed(rootMotionSpeed); }

	//	----- Collision -----
	virtual void RegisterCollisionData() = 0;
	virtual void UpdateCollisions(const float& elapsedTime);
	void SetAllAttackDetectionActiveFlag(const bool& isActive);	//	攻撃判定の有効フラグを全て変更する
	void CollisionCharacterVsStage();
	
	//	----- 攻撃判定 -----
	void RegisterAttackDetectionData(const AttackDetectionData& data);
	const int GetAttackDetectionDataCount() const { return static_cast<int>(attackDetectionData_.size()); }
	std::vector<AttackDetectionData> GetAttackDetectionData() { return attackDetectionData_; }
	AttackDetectionData&	GetAttackDetectionData(const std::string& name);
	AttackDetectionData&	GetAttackDetectionData(const int& index);
	const float				GetRadius()	const{ return radius_; }
	const float				GetHeight()	const{ return height_; }

	//	----- くらい判定 -----
	void RegisterDamageDetectionData(const DamageDetectionData& data);
	const int GetDamageDetectionDataCount() const { return static_cast<int>(damageDetectionData_.size()); }
	std::vector<DamageDetectionData> GetDamageDetectionData() { return damageDetectionData_; }
	DamageDetectionData& GetDamageDetectionData(const std::string& name);
	DamageDetectionData& GetDamageDetectionData(const int& index);

	//	----- 押し出し判定 -----
	void RegisterCollisionDetectionData(const CollisionDetectionData& data);
	const int GetCollisionDetectionDataCount() const { return static_cast<int>(collisionDetectionData_.size()); }
	std::vector<CollisionDetectionData> GetCollisionDetectionData() { return collisionDetectionData_; }
	CollisionDetectionData& GetCollisionDetectionData(const std::string& name);
	CollisionDetectionData& GetCollisionDetectionData(const int& index);

	//	----- 攻撃力 -----
	void		SetAttackPower(const float& attackPower) { attackPower_ = attackPower; }
	const float GetAttackPower()const { return attackPower_; }

	//	----- 吹っ飛ばし -----
	float				blowPower_ = 0.0f;			//	吹っ飛ばす力
	DirectX::XMFLOAT3	blowDirection_ = {};		//	吹っ飛ばす方向
	float				decelerationForce_ = 0.0f;	//	1フレームでどれくらい力を減衰させるか

	Transform* GetTransform() { return gltfModelResource_->GetTransform(); }

protected:
	//	----- 移動 -----
	DirectX::XMFLOAT3	velocity_		= {};	//	移動速度
	DirectX::XMFLOAT3	acceleration_	= {};	//	加速度(0なら等速直線運動)
	DirectX::XMFLOAT3	moveVec_		= {};	//	移動ベクトル
	float				moveSpeed_		= 4.0f;								//	移動する速さ
	float				defaultMoveSpeed_ = 4.0f;							//	通常時の移動する速さ

	//	----- 旋回処理 -----
	bool	isTurnAction_	= true;								//	旋回処理するかどうか
	float	turnSpeed_		= DirectX::XMConvertToRadians(720);	//	旋回する速さ
	
	//	----- 無敵処理 -----
	bool	isInvincible_	= false;		//	無敵かどうか
	float	invincibleTimer_ = 0.0f;		//	無敵時間

	//	----- Collision -----
	std::vector<AttackDetectionData>	attackDetectionData_;		//	攻撃判定用
	std::vector<DamageDetectionData>	damageDetectionData_;		//	くらい判定
	std::vector<CollisionDetectionData>	collisionDetectionData_;	//	押し出し判定用
	float	radius_ = 30.0f;	//	半径
	float	height_ = 195.0f;	//	高さ

	int		hp_ = 100;				//	現在のHP
	bool	isDead_ = false;		//	死亡フラグ
	float	attackPower_ = 5.0f;	//	攻撃力

private:
	const float MoveSpeed_ = 20.0f;	//	最大の速さ

private:
	std::shared_ptr <GltfModel>					gltfModelResource_;		//	Gltfモデル
	Microsoft::WRL::ComPtr <ID3D11PixelShader>	pixelShader_;

};


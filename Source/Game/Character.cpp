#include "Character.h"

#include "Stage.h"
#include "../Nova/Core/Framework.h"
#include "../Nova/Others/MathHelper.h"

//	コンストラクタ
Character::Character(const std::string& filename, const std::string& rootNodeName)
{
	//	モデル読み込み
	gltfModelResource_ = ResourceManager::Instance().LoadGltfModelResource(filename, rootNodeName);
}

//	更新処理
void Character::Update(const float& elapsedTime)
{
	//	----- 吹っ飛ばし処理更新 -----
	UpdateForce(elapsedTime);

}

//	ベロシティ更新
void Character::UpdateVelocity(const float& elapsedTime)
{
	//#if 0 //	キャラの前方向に移動させる処理
	//	DirectX::XMFLOAT3 front = this->GetTransform()->CalcForward();	//	キャラクターの前方向取得
	//	DirectX::XMVECTOR frontV = DirectX::XMLoadFloat3(&front);
	//	frontV = DirectX::XMVector3Normalize(frontV);
	//	DirectX::XMStoreFloat3(&front, frontV);
	//
	//	float range = 10.0f;
	//	velocity_ = front * range * Framework::tictoc_.GetDeltaTime();
	//	this->GetTransform()->AddPosition(velocity_);
	//#endif

#if 0
	float vecLength = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMLoadFloat3(&moveVec_)));
	if (vecLength > 0)
	{
		velocity_ = moveVec_ * moveSpeed_ * elapsedTime;
	}
#else
	velocity_ = moveVec_ * moveSpeed_ * elapsedTime + acceleration_ * elapsedTime;

#endif
}

//	ベロシティ加算
void Character::AddVelocity(const DirectX::XMFLOAT3& addVelocity,const float& elapsedTime)
{
	DirectX::XMVECTOR Velocity = DirectX::XMLoadFloat3(&velocity_);
	DirectX::XMVECTOR AddVelocity = DirectX::XMLoadFloat3(&addVelocity);
	Velocity = DirectX::XMVectorAdd(Velocity, AddVelocity);
	//Velocity = DirectX::XMVectorScale(Velocity, elapsedTime);
	DirectX::XMStoreFloat3(&velocity_, Velocity);
}

//	VelocityのY方向のみ加算する
void Character::AddVelocityY(const float& addVelocityY, const float& elapsedTime)
{
	velocity_.y += addVelocityY * elapsedTime;
}

//	VelocityのXZ方向のみ加算する
void Character::AddVelocityXZ(const float& addVelocityX, const float& addVelocityZ, const float& elapsedTime)
{
	velocity_.x += addVelocityX * elapsedTime;
	velocity_.z += addVelocityZ * elapsedTime;
}

//	VelocityのXZ方向のみ乗算する
void Character::MultiplyVelocityXZ(const float& multiplyVelocity, const float& elapsedTime)
{
	velocity_.x *= multiplyVelocity * elapsedTime;
	velocity_.z *= multiplyVelocity * elapsedTime;
}

//	加速度加算
void Character::AddAcceleration(const DirectX::XMFLOAT3& addAcceleration, const float& elapsedTime)
{
	DirectX::XMVECTOR Acceleration = DirectX::XMLoadFloat3(&acceleration_);
	DirectX::XMVECTOR AddAcceleration = DirectX::XMVectorScale(DirectX::XMLoadFloat3(&addAcceleration), elapsedTime);
	Acceleration = DirectX::XMVectorAdd(Acceleration, AddAcceleration);
	DirectX::XMStoreFloat3(&acceleration_, Acceleration);
}

void Character::AddAccelerationY(const float& addAccelerationY, const float& elapsedTime)
{
	acceleration_.y += addAccelerationY * elapsedTime;
}

void Character::AddAccelerationXZ(const float& addAccelerationX, const float& addAccelerationZ, const float& elapsedTime)
{
	acceleration_.x += addAccelerationX * elapsedTime;
	acceleration_.z += addAccelerationZ * elapsedTime;
}

void Character::AddMoveSpeed(const float& addMoveSpeed, const float& elapsedTime)
{
	//	最大スピードを超えていない場合のみ加算処理
	if (moveSpeed_ < MoveSpeed_)
	{
		moveSpeed_ += addMoveSpeed * elapsedTime;
	}
	else if (MoveSpeed_ >= moveSpeed_)
	{
		moveSpeed_ = MoveSpeed_;
	}
}

//	吹っ飛ばし処理更新
void Character::UpdateForce(const float& elapsedTime)
{
	// パワーが無いときは処理しない
	if (blowPower_ <= 0) return;

	blowPower_ -= decelerationForce_ * elapsedTime;
	blowPower_ = std::max(blowPower_, 0.0f); // 0.0f以下にならないようにする

	// 吹っ飛び方向にどれだけ、吹っ飛ばすかを計算する
	DirectX::XMFLOAT3 direction = {};
	direction = Normalize(blowDirection_) * blowPower_ * elapsedTime;

	// 吹っ飛ばす。
	GetTransform()->AddPosition(direction);
}

//	吹っ飛ばし
void Character::AddForce(const DirectX::XMFLOAT3& direction, const float& power, const float& decelerationForce)
{
	blowDirection_ = direction;
	blowPower_ = power;
	decelerationForce_ = decelerationForce;
}

//	移動処理
void Character::Move(const float& elapsedTime)
{
	this->GetTransform()->AddPosition(velocity_);
}

//	旋回処理
void Character::Turn(const float& elapsedTime, float vx, float vz, float speed)
{
	speed *= elapsedTime;

	//	進行ベクトルがゼロベクトルの場合は処理する必要なし
	float length;
	length = sqrtf(vx * vx + vz * vz);

	if (length <= 0)
	{
		return;
	}

	//	進行ベクトルを単位ベクトル化
	vx /= length;
	vz /= length;

	//	自身の回転値から前方向を求める
	float angleY = this->GetTransform()->GetRotationY();
	float frontX = sinf(angleY);
	float frontZ = cosf(angleY);

	//	左右判定を行うために２つの単位ベクトルの外積を計算する
	float cross = (frontZ * vx) - (frontX * vz);

	//	回転角を求めるため、２つの単位ベクトルの内積を計算する
	float dot = (frontX * vx) + (frontZ * vz);

	//	内積値は-1.0～1.0で表現されている。
	//	２つの単位ベクトルの角度が小さいほど
	//	1.0に近づくという性質を利用して回転速度を調整する
	float rot = 1.0f - dot;
	if (rot > speed)rot = speed;

	//	2Dの外積値が正の場合か負の場合によって左右判定が行える
	//	左右判定を行うことによって左右回転を選択する
	//	正の場合は右、負の場合は左
	if (cross < 0.0f)//	左
	{
		this->GetTransform()->AddRotationY(-rot);
	}
	else//	右
	{
		this->GetTransform()->AddRotationY(rot);
	}
}

//	アニメーション再生
void Character::PlayAnimation(const int& index, const bool& loop, const float& blendTime, const float& animSpeed, const float& startFrame, const float& endFrame)
{
	gltfModelResource_->PlayAnimation(index, loop, blendTime, animSpeed, startFrame, endFrame);
}

//	アニメーション更新処理
void Character::UpdateAnimation(const float& elapsedTime)
{
	gltfModelResource_->UpdateAnimation(elapsedTime);
}

//	アニメーション再生中かどうか
bool Character::IsPlayAnimation()const
{
	return gltfModelResource_->IsPlayAnimation();
}

//	ピクセルシェーダー設定
void Character::SetPixelShader(const char* csoName)
{
	ID3D11Device* device = Graphics::Instance().GetDevice();
	Graphics::Instance().GetShader()->CreatePsFromCso(device, csoName, pixelShader_.ReleaseAndGetAddressOf());
	gltfModelResource_->SetPixelShader(pixelShader_.Get());
}

//	名前からジョイントポジション取得
DirectX::XMFLOAT3 Character::GetJointPosition(const std::string& nodeName, const DirectX::XMFLOAT3& offsetPos)
{
	DirectX::XMFLOAT4X4 transform = {};
	DirectX::XMStoreFloat4x4(&transform, gltfModelResource_->GetTransform()->CalcWorld());
	return gltfModelResource_->GetJointPosition(nodeName, transform, offsetPos);
}

//	登録番号からジョイントポジション取得
DirectX::XMFLOAT3 Character::GetJointPosition(const size_t& nodeIndex, const DirectX::XMFLOAT3& offsetPos)
{
	DirectX::XMFLOAT4X4 transform = {};
	DirectX::XMStoreFloat4x4(&transform, gltfModelResource_->GetTransform()->CalcWorld());
	return gltfModelResource_->GetJointPosition(nodeIndex, transform, offsetPos);
}

//	HP減少
void Character::SubtractHp(const int& hp)
{
	//	無敵でなければHP減少
	if (isInvincible_ == false)	
	{
		hp_ -= hp;
		if (hp_ <= 0)
		{
			hp_ = 0;
		}
	}
}

//	アニメーション追加
void Character::AppendAnimation(const std::string& filename)
{
	gltfModelResource_->AppendAnimation(filename);
}

//	========== Collision ==========
#pragma region //	========== Collision ========== 
void Character::UpdateCollisions(const float& elapsedTime)
{
	//	攻撃判定更新
	for (AttackDetectionData& data : attackDetectionData_)
	{
		// ジョイントの名前で位置設定(名前がジョイントの名前ではないとき別途更新必要)
		data.SetJointPosition(GetJointPosition(data.GetUpdateName(), data.GetOffsetPosition()));
	}
	//	くらい判定更新
	for (DamageDetectionData& data : damageDetectionData_)
	{
		// ジョイントの名前で位置設定(名前がジョイントの名前ではないとき別途更新必要)
		data.SetJointPosition(GetJointPosition(data.GetUpdateName(), data.GetOffsetPosition()));

		data.Update(elapsedTime);
	}
	// 押し出し判定更新
	for (CollisionDetectionData& data : collisionDetectionData_)
	{
		// ジョイントの名前で位置設定(名前がジョイントの名前ではないとき別途更新必要)
		data.SetJointPosition(GetJointPosition(data.GetUpdateName(), data.GetOffsetPosition()));
	}
}

//	攻撃判定の有効フラグをすべて設定する
void Character::SetAllAttackDetectionActiveFlag(const bool& isActive)
{
	for (AttackDetectionData& data : attackDetectionData_)
	{
		data.SetIsActive(isActive);
	}
}

#pragma region ----- 攻撃判定 ----- 
//	攻撃判定用データ登録
void Character::RegisterAttackDetectionData(const AttackDetectionData& data)
{
	attackDetectionData_.emplace_back(data);
}

//	名前からデータを取得
AttackDetectionData& Character::GetAttackDetectionData(const std::string& name)
{
	//	名前でデータを探す
	for (AttackDetectionData& data : attackDetectionData_)
	{
		if (data.GetName() != name) continue;

		return data;
	}

	//	見つからなかった
	_ASSERT_EXPR(false, "not found AttackDetectionData");
	return AttackDetectionData();
}

//	登録番号からデータを取得
AttackDetectionData& Character::GetAttackDetectionData(const int& index)
{
	return attackDetectionData_.at(index);
}

#pragma endregion ----- 攻撃判定 ----- 

#pragma region ----- くらい判定 ----- 
//	くらい判定用データ登録
void Character::RegisterDamageDetectionData(const DamageDetectionData& data)
{
	damageDetectionData_.emplace_back(data);
}
//	名前からデータを取得
DamageDetectionData& Character::GetDamageDetectionData(const std::string& name)
{
	//	名前でデータを探す
	for (DamageDetectionData& data : damageDetectionData_)
	{
		if (data.GetName() != name) continue;

		return data;
	}

	//	見つからなかった
	_ASSERT_EXPR(false, "not found DamageDetectionData");
	return DamageDetectionData();
}

//	登録番号からデータを取得
DamageDetectionData& Character::GetDamageDetectionData(const int& index)
{
	return damageDetectionData_.at(index);
}

#pragma endregion ----- くらい判定 ----- 

#pragma region ----- 押し出し判定 ----- 
//	押し出し判定用データ登録
void Character::RegisterCollisionDetectionData(const CollisionDetectionData& data)
{
	collisionDetectionData_.emplace_back(data);
}

//	名前からデータを取得
CollisionDetectionData& Character::GetCollisionDetectionData(const std::string& name)
{
	//	名前でデータを探す
	for (CollisionDetectionData& data : collisionDetectionData_)
	{
		if (data.GetName() != name) continue;

		return data;
	}

	//	見つからなかった
	_ASSERT_EXPR(false, "not found CollisionDetectionData");
	return CollisionDetectionData();
}

//	登録番号からデータを取得
CollisionDetectionData& Character::GetCollisionDetectionData(const int& index)
{
	return collisionDetectionData_.at(index);
}

#pragma endregion ----- 押し出し判定 ----- 
#pragma endregion //	========== Collision ========== 

//	描画処理
void Character::Render()
{
	gltfModelResource_->Render(GetTransform()->CalcWorld());
}

//	デバッグ描画
void Character::DrawDebug()
{
	//	----- トランスフォーム -----
	GetTransform()->DrawDebug();
	//	----- アニメーション -----
	gltfModelResource_->DrawDebug();

	//	----- HP -----
	ImGui::Checkbox("Invincible", &isInvincible_);				//	無敵フラグ設定
	ImGui::DragInt("HP", &hp_, 1.0f, 0, INT_MAX);
	//	----- 移動 -----
	ImGui::DragFloat3("Velocity", &velocity_.x, 0.01f, -FLT_MAX, FLT_MAX);			//	移動速度
	ImGui::DragFloat3("Acceleration", &acceleration_.x, 0.01f, -FLT_MAX, FLT_MAX);	//	加速度
	ImGui::DragFloat3("moveVec", &moveVec_.x, 0.01f, -FLT_MAX, FLT_MAX);			//	移動ベクトル
	ImGui::DragFloat("MoveSpeed", &moveSpeed_, 0.01f, 0.0f, FLT_MAX);				//	移動する速さ
	ImGui::DragFloat("DefaultMoveSpeed", &defaultMoveSpeed_, 0.01f, 0.0f, FLT_MAX);	//	デフォルトの移動する速さ

	//	----- 旋回 -----
	ImGui::Checkbox("Turn Action", &isTurnAction_);			//	旋回するかどうか
	ImGui::DragFloat("TurnSpeed", &turnSpeed_, 0.01f);		//	旋回速度

	//	----- 当たり判定の大きさ -----
	ImGui::DragFloat("Height", &height_, 0.01f, -FLT_MAX, FLT_MAX);					//	高さ
	ImGui::DragFloat("Radius", &radius_, 0.01f, -FLT_MAX, FLT_MAX);					//	半径

	//	----- Collision -----
	if (ImGui::TreeNode("Collision"))
	{
		if (ImGui::TreeNode("DamageDetection"))
		{
			for (DamageDetectionData& data : damageDetectionData_)
			{
				data.DrawDebug();
			}
			if (ImGui::Button("Add"))
			{
				RegisterDamageDetectionData({});
			}
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("AttackDetection"))
		{
			for (AttackDetectionData& data : attackDetectionData_)
			{
				//if (data.GetIsActive() == false) continue;
				data.DrawDebug();
			}
			if (ImGui::Button("Add"))
			{
				RegisterAttackDetectionData({});
			}
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("CollisionDetection"))
		{
			for (CollisionDetectionData& data : collisionDetectionData_)
			{
				data.DrawDebug();
			}
			if (ImGui::Button("Add"))
			{
				RegisterCollisionDetectionData({});
			}
			ImGui::TreePop();
		}
		ImGui::TreePop();
	}
}


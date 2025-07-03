#include "Character.h"

#include <json.hpp>
#include <iostream>
#include <fstream>

#include "../Stage/Stage.h"
#include "../../Nova/Core/Framework.h"
#include "../../Nova/Others/MathHelper.h"
#include "../../Nova/Resources/ResourceManager.h"

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
	velocity_ = moveVec_ * moveSpeed_ * elapsedTime + acceleration_ * elapsedTime;
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

//	速度をY方向のみ加算する
void Character::AddVelocityY(const float& addVelocityY, const float& elapsedTime)
{
	velocity_.y += addVelocityY * elapsedTime;
}

//	速度をXZ方向のみ加算する
void Character::AddVelocityXZ(const float& addVelocityX, const float& addVelocityZ, const float& elapsedTime)
{
	velocity_.x += addVelocityX * elapsedTime;
	velocity_.z += addVelocityZ * elapsedTime;
}

//	速度をXZ方向のみ乗算する
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

//	加速度をY方向のみ加算する
void Character::AddAccelerationY(const float& addAccelerationY, const float& elapsedTime)
{
	acceleration_.y += addAccelerationY * elapsedTime;
}

//	加速度をXZ方向のみ加算する
void Character::AddAccelerationXZ(const float& addAccelerationX, const float& addAccelerationZ, const float& elapsedTime)
{
	acceleration_.x += addAccelerationX * elapsedTime;
	acceleration_.z += addAccelerationZ * elapsedTime;
}

//	移動スピード加算
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
	//	パワーが無いときは処理しない
	if (blowPower_ <= 0.0f) return;

	//	吹っ飛ばす力更新
	blowPower_ -= decelerationForce_ * elapsedTime;
	blowPower_ = std::max(blowPower_, 0.0f); // 0.0f未満にならないようにする

	//	吹っ飛び方向にどれだけ吹っ飛ばすかを計算する
	DirectX::XMFLOAT3 direction = {};
	direction = Normalize(blowDirection_) * blowPower_ * elapsedTime;

	//	吹っ飛ばす
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
void Character::UpdatePosition(const float& elapsedTime)
{
	GetTransform()->AddPosition(velocity_);
}

//	旋回処理
void Character::Turn(const float& elapsedTime, float vx, float vz, float speed)
{
	speed *= elapsedTime;

	//	進行ベクトルがゼロベクトルの場合は処理する必要なし
	float length;
	length = sqrtf(vx * vx + vz * vz);
	if (length <= 0.0f)
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
void Character::SubtractHp(const float& hp)
{
	//	無敵でなければHP減少
	if (isInvincible_ == false)	
	{
		hp_ -= hp;
		if (hp_ <= 0.0f)
		{
			hp_ = 0.0f;
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

#pragma region ----- 攻撃判定 ----- 
//	攻撃判定の有効フラグをすべて設定する
void Character::SetAllAttackDetectionActiveFlag(const bool& isActive)
{
	for (AttackDetectionData& data : attackDetectionData_)
	{
		data.SetIsActive(isActive);
	}
}

//	攻撃判定用データ登録
void Character::RegisterAttackDetectionData(const AttackDetectionData& data)
{
	attackDetectionData_.emplace_back(data);
}

//	名前からデータを取得
AttackDetectionData* Character::GetAttackDetectionData(const std::string& name)
{
	//	名前でデータを探す
	for (AttackDetectionData& data : attackDetectionData_)
	{
		if (data.GetName() != name) continue;

		return &data;
	}

	//	見つからなかった
	_ASSERT_EXPR(false, "not found AttackDetectionData");
	return nullptr;
}

//	登録番号からデータを取得
AttackDetectionData* Character::GetAttackDetectionData(const int& index)
{
	//	インデックスが有効範囲内かチェック
	if (index < 0 || static_cast<size_t>(index) >= attackDetectionData_.size())
	{
		//	範囲外だった
		_ASSERT_EXPR(false, "Invalid index for AttackDetectionData");
		return nullptr;
	}

	return &attackDetectionData_[index];
}

#pragma endregion ----- 攻撃判定 ----- 

#pragma region ----- くらい判定 ----- 
//	くらい判定用データ登録
void Character::RegisterDamageDetectionData(const DamageDetectionData& data)
{
	damageDetectionData_.emplace_back(data);
}
//	名前からデータを取得
DamageDetectionData* Character::GetDamageDetectionData(const std::string& name)
{
	//	名前でデータを探す
	for (DamageDetectionData& data : damageDetectionData_)
	{
		if (data.GetName() != name) continue;

		return &data;
	}

	//	見つからなかった
	_ASSERT_EXPR(false, "not found DamageDetectionData");
	return nullptr;
}

//	登録番号からデータを取得
DamageDetectionData* Character::GetDamageDetectionData(const int& index)
{
	//	インデックスが有効範囲内かチェック
	if (index < 0 || static_cast<size_t>(index) >= damageDetectionData_.size())
	{
		//	範囲外だった
		_ASSERT_EXPR(false, "Invalid index for DamageDetectionData");
		return nullptr;
	}

	return &damageDetectionData_[index];
}

#pragma endregion ----- くらい判定 ----- 

#pragma region ----- 押し出し判定 ----- 
//	押し出し判定用データ登録
void Character::RegisterCollisionDetectionData(const CollisionDetectionData& data)
{
	collisionDetectionData_.emplace_back(data);
}

//	名前からデータを取得
CollisionDetectionData* Character::GetCollisionDetectionData(const std::string& name)
{
	//	名前でデータを探す
	for (CollisionDetectionData& data : collisionDetectionData_)
	{
		if (data.GetName() != name) continue;

		return &data;
	}

	//	見つからなかった
	_ASSERT_EXPR(false, "not found CollisionDetectionData");
	return nullptr;
}

//	登録番号からデータを取得
CollisionDetectionData* Character::GetCollisionDetectionData(const int& index)
{
	//	インデックスが有効範囲内かチェック
	if (index < 0 || static_cast<size_t>(index) >= collisionDetectionData_.size())
	{
		//	範囲外だった
		_ASSERT_EXPR(false, "Invalid index for CollisionDetectionData");
		return nullptr;
	}

	return &collisionDetectionData_[index];
}

#pragma endregion ----- 押し出し判定 ----- 
#pragma endregion //	========== Collision ========== 

//	Json(当たり判定データ)保存
void Character::SaveCollisionDataToJson(const std::string& filePath) const
{
	nlohmann::json j;
	j["collisionDetectionData"] = collisionDetectionData_;
	j["attackDetectionData"] = attackDetectionData_;
	j["damageDetectionData"] = damageDetectionData_;

	std::ofstream ofs(filePath);
	if (ofs.is_open())
	{
		ofs << std::setw(4) << j << std::endl; // 整形して保存 (インデント4)
		ofs.close();

	}
	else 
	{
		//	保存失敗
		_ASSERT_EXPR(false, "Failed to open file for saving");
	}
}

//	Json(当たり判定データ)読み込み
void Character::LoadCollisionDataFromJson(const std::string& filePath)
{
	std::ifstream ifs(filePath);
	if (ifs.is_open()) 
	{
		nlohmann::json j;
		ifs >> j;
		ifs.close();

		//	各データが存在するか確認し、存在すればロード
		if (j.contains("collisionDetectionData"))
		{
			j.at("collisionDetectionData").get_to(collisionDetectionData_);
		}
		if (j.contains("attackDetectionData"))
		{
			j.at("attackDetectionData").get_to(attackDetectionData_);
		}
		if (j.contains("damageDetectionData")) 
		{
			j.at("damageDetectionData").get_to(damageDetectionData_);
		}
	}
	else 
	{
		//	読み込み失敗
		_ASSERT_EXPR(false, "File not found or failed to open for loading");
	}
}

//	描画処理
void Character::Render()
{
	gltfModelResource_->Render();
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
	ImGui::DragFloat("HP", &hp_, 1.0f, 0, FLT_MAX);
	//	----- 移動 -----
	ImGui::DragFloat3("Velocity", &velocity_.x, 0.01f, -FLT_MAX, FLT_MAX);			//	移動速度
	ImGui::DragFloat3("Acceleration", &acceleration_.x, 0.01f, -FLT_MAX, FLT_MAX);	//	加速度
	ImGui::DragFloat3("moveVec", &moveVec_.x, 0.01f, -FLT_MAX, FLT_MAX);			//	移動ベクトル
	ImGui::DragFloat("MoveSpeed", &moveSpeed_, 0.01f, 0.0f, FLT_MAX);				//	移動する速さ
	ImGui::DragFloat("DefaultMoveSpeed", &defaultMoveSpeed_, 0.01f, 0.0f, FLT_MAX);	//	デフォルトの移動する速さ

	//	----- 旋回 -----
	ImGui::Checkbox("TurnAction", &isTurnAction_);			//	旋回するかどうか
	ImGui::DragFloat("TurnSpeed", &turnSpeed_, 0.01f);		//	旋回速度

	//	----- 当たり判定の大きさ -----
	ImGui::DragFloat("Height", &height_, 0.01f, -FLT_MAX, FLT_MAX);					//	高さ
	ImGui::DragFloat("Radius", &radius_, 0.01f, -FLT_MAX, FLT_MAX);					//	半径

	//	----- Collision -----
	if (ImGui::TreeNode(u8"Collision 当たり判定"))
	{
		//	当たり判定表示フラグ
		ImGui::Text(u8"CollisionDrawFlag 当たり判定表示フラグ");
		ImGui::Checkbox("IsDrawDamageSphere", &isDrawDamageSphere_);		//	くらい判定
		ImGui::Checkbox("IsDrawAttackSphere", &isDrawAttackSphere_);		//	攻撃判定
		ImGui::Checkbox("IsDrawCollisionSphere", &isDrawCollisionSphere_);	//	押し出し判定

		CollisionDrawDebug();
		ImGui::TreePop();
	}

}

//	当たり判定データImGui
void Character::CollisionDrawDebug()
{
	//	----- Collision -----
	if (ImGui::TreeNode(u8"Collision 当たり判定"))
	{
		//	JSONファイル名表示とボタン
		ImGui::Text("JSON File: %s", collisionDataJsonFileName_.c_str());

		//	JSON保存
		if (ImGui::Button(u8"Save JSON"))
		{
			SaveCollisionDataToJson(collisionDataJsonFileName_);
		}
		ImGui::SameLine();
		//	JSON読み込み
		if (ImGui::Button(u8"Load JSON"))
		{
			LoadCollisionDataFromJson(collisionDataJsonFileName_);
		}
		ImGui::Separator(); // 区切り線

		//	手動でのインデックス入力/表示
		ImGui::InputInt("AttackDetectionDataIndex", &selectedAttackDetectionDataIndex_, 1);
		ImGui::InputInt("DamageDetectionDataIndex", &selectedDamageDetectionDataIndex_, 1);
		ImGui::InputInt("CollisionDetectionDataIndex", &selectedCollisionDetectionDataIndex_, 1);
		ImGui::Separator();

		//	DamageDetection くらい判定
		if (ImGui::TreeNode(u8"DamageDetection くらい判定"))
		{
			//	選択したデータを複製
			if (ImGui::Button(u8"Duplicate Selected DamageDetection 複製"))
			{
				if (selectedDamageDetectionDataIndex_ != -1 && selectedDamageDetectionDataIndex_ < damageDetectionData_.size())
				{
					damageDetectionData_.insert(damageDetectionData_.begin() + selectedDamageDetectionDataIndex_ + 1,
						damageDetectionData_[selectedDamageDetectionDataIndex_]);
					selectedDamageDetectionDataIndex_++;
				}
			}

			//	各DamageDetectionDataのループ
			for (int i = 0; i < damageDetectionData_.size(); ++i)
			{
				ImGui::PushID(i); // ループ内のアイテムにユニークなIDをプッシュ

				std::string nodeName = "DamageDetection [" + std::to_string(i) + "] " + damageDetectionData_[i].GetName();

				ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_None;
				//	現在のアイテムが選択されている場合、TreeNodeのヘッダーをハイライト表示
				if (selectedDamageDetectionDataIndex_ == i) 
				{
					nodeFlags |= ImGuiTreeNodeFlags_Selected;
				}

				//	TreeNodeEx を使って、展開可能かつ選択可能な項目として表示
				bool nodeOpen = ImGui::TreeNodeEx(nodeName.c_str(), nodeFlags);

				//	TreeNodeのヘッダーがクリックされたら、そのアイテムを選択状態にする
				if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
				{
					selectedDamageDetectionDataIndex_ = i;
				}

				if (nodeOpen)
				{
					//	個別の判定データのImGuiを描画
					damageDetectionData_[i].DrawDebug();

					//	削除ボタン
					ImGui::PushID("Delete");	//	削除ボタンにユニークなIDをプッシュ
					if (ImGui::Button("Delete"))
					{
						damageDetectionData_.erase(damageDetectionData_.begin() + i);
						if (selectedDamageDetectionDataIndex_ == i)		//	削除されたアイテムが選択されていたら選択解除
						{
							selectedDamageDetectionDataIndex_ = -1;
						}
						else if (selectedDamageDetectionDataIndex_ > i) //	削除されたアイテムより後ろのアイテムが選択されていたらインデックスを調整
						{
							selectedDamageDetectionDataIndex_--;
						}
						ImGui::PopID();		//	"Delete" IDをポップ
						ImGui::TreePop();	//	現在のTreeNodeをポップ
						ImGui::PopID();		//	アイテムIDをポップ
						--i;				//	要素が削除されたのでループのインデックスを調整
						continue;			//	このイテレーションの残りをスキップ
					}
					ImGui::PopID();		//	"Delete" IDをポップ

					ImGui::TreePop();	//	現在のTreeNodeをポップ
				}
				ImGui::PopID();	//	アイテムIDをポップ
			}
			ImGui::TreePop();	//	"DamageDetection くらい判定" TreeNodeをポップ
		}

		//	AttackDetection 攻撃判定
		if (ImGui::TreeNode(u8"AttackDetection 攻撃判定"))
		{
			//	選択したデータを複製
			if (ImGui::Button(u8"Duplicate Selected AttackDetection 複製"))
			{
				if (selectedAttackDetectionDataIndex_ != -1 && selectedAttackDetectionDataIndex_ < attackDetectionData_.size())
				{
					attackDetectionData_.insert(attackDetectionData_.begin() + selectedAttackDetectionDataIndex_ + 1,
						attackDetectionData_[selectedAttackDetectionDataIndex_]);
					selectedAttackDetectionDataIndex_++;
				}
			}

			//	各AttackDetectionDataのループ
			for (int i = 0; i < attackDetectionData_.size(); ++i)
			{
				ImGui::PushID(i);

				std::string nodeName = "AttackDetection [" + std::to_string(i) + "] " + attackDetectionData_[i].GetName();

				ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_None;
				//	現在のアイテムが選択されている場合、TreeNodeのヘッダーをハイライト表示
				if (selectedAttackDetectionDataIndex_ == i)
				{
					nodeFlags |= ImGuiTreeNodeFlags_Selected;
				}

				//	TreeNodeEx を使って、展開可能かつ選択可能な項目として表示
				bool nodeOpen = ImGui::TreeNodeEx(nodeName.c_str(), nodeFlags);

				//	TreeNodeのヘッダーがクリックされたら、そのアイテムを選択状態にする
				if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
				{
					selectedAttackDetectionDataIndex_ = i;
				}

				if (nodeOpen)
				{
					//	個別の判定データのImGuiを描画
					attackDetectionData_[i].DrawDebug();

					//	削除ボタン
					ImGui::PushID("Delete");
					if (ImGui::Button("Delete"))
					{
						attackDetectionData_.erase(attackDetectionData_.begin() + i);
						if (selectedAttackDetectionDataIndex_ == i)		//	削除されたアイテムが選択されていたら選択解除
						{
							selectedAttackDetectionDataIndex_ = -1;
						}
						else if (selectedAttackDetectionDataIndex_ > i)	//	削除されたアイテムより後ろのアイテムが選択されていたらインデックスを調整
						{
							selectedAttackDetectionDataIndex_--;
						}
						ImGui::PopID();		//	"Delete" IDをポップ
						ImGui::TreePop();	//	現在のTreeNodeをポップ
						ImGui::PopID();		//	アイテムIDをポップ
						--i;				//	要素が削除されたのでループのインデックスを調整
						continue;			//	このイテレーションの残りをスキップ
					}
					ImGui::PopID();		//	"Delete" IDをポップ

					ImGui::TreePop();	//	現在のTreeNodeをポップ
				}
				ImGui::PopID();	//	アイテムIDをポップ
			}
			ImGui::TreePop();	//	"AttackDetection 攻撃判定" TreeNodeをポップ
		}

		//	CollisionDetection 押し出し判定
		if (ImGui::TreeNode(u8"CollisionDetection 押し出し判定"))
		{
			//	選択したデータを複製
			if (ImGui::Button(u8"Duplicate Selected CollisionDetection 複製"))
			{
				if (selectedCollisionDetectionDataIndex_ != -1 && selectedCollisionDetectionDataIndex_ < collisionDetectionData_.size())
				{
					collisionDetectionData_.insert(collisionDetectionData_.begin() + selectedCollisionDetectionDataIndex_ + 1,
						collisionDetectionData_[selectedCollisionDetectionDataIndex_]);
					selectedCollisionDetectionDataIndex_++;
				}
			}

			//	各CollisionDetectionDataのループ
			for (int i = 0; i < collisionDetectionData_.size(); ++i)
			{
				ImGui::PushID(i);

				std::string nodeName = "CollisionDetection [" + std::to_string(i) + "] " + collisionDetectionData_[i].GetName();

				ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_None;
				//	現在のアイテムが選択されている場合、TreeNodeのヘッダーをハイライト表示
				if (selectedCollisionDetectionDataIndex_ == i) 
				{
					nodeFlags |= ImGuiTreeNodeFlags_Selected;
				}

				//	TreeNodeEx を使って、展開可能かつ選択可能な項目として表示
				bool nodeOpen = ImGui::TreeNodeEx(nodeName.c_str(), nodeFlags);

				//	TreeNodeのヘッダーがクリックされたら、そのアイテムを選択状態にする
				if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
				{
					selectedCollisionDetectionDataIndex_ = i;
				}

				if (nodeOpen)
				{
					//	個別の判定データのImGuiを描画
					collisionDetectionData_[i].DrawDebug();

					//	削除ボタン
					ImGui::PushID("Delete");
					if (ImGui::Button("Delete"))
					{
						collisionDetectionData_.erase(collisionDetectionData_.begin() + i);
						if (selectedCollisionDetectionDataIndex_ == i)		//	削除されたアイテムが選択されていたら選択解除
						{
							selectedCollisionDetectionDataIndex_ = -1;
						}
						else if (selectedCollisionDetectionDataIndex_ > i)	//	削除されたアイテムより後ろのアイテムが選択されていたらインデックスを調整
						{
							selectedCollisionDetectionDataIndex_--;
						}
						ImGui::PopID();		//	"Delete" IDをポップ
						ImGui::TreePop();	//	現在のTreeNodeをポップ
						ImGui::PopID();		//	アイテムIDをポップ
						--i;				//	要素が削除されたのでループのインデックスを調整
						continue;			//	このイテレーションの残りをスキップ
					}
					ImGui::PopID();		//	"Delete" IDをポップ

					ImGui::TreePop();	//	現在のTreeNodeをポップ
				}
				ImGui::PopID();	//	アイテムIDをポップ
			}
			ImGui::TreePop();	//	"AttackDetection 攻撃判定" TreeNodeをポップ
		}
		ImGui::TreePop(); // "Collision 当たり判定" TreeNodeをポップ
	}
}

//	デバッグプリミティブ描画
void Character::DrawDebugPrimitive()
{
	DebugRenderer* debugRenderer = Graphics::Instance().GetDebugRenderer();

	//	----- Collision -----
	if (isDrawCollisionSphere_)
	{
		for (auto& data : GetCollisionDetectionData())
		{
			//	現在アクティブではないので表示しない
			if (data.GetIsActive() == false) continue;

			debugRenderer->DrawSphere(data.GetPosition(), data.GetRadius(), data.GetColor());
		}
	}
	if (isDrawDamageSphere_)
	{
		for (auto& data : GetDamageDetectionData())
		{
			debugRenderer->DrawSphere(data.GetPosition(), data.GetRadius(), data.GetColor());
		}
	}
	if (isDrawAttackSphere_)
	{
		for (auto& data : GetAttackDetectionData())
		{
			//	現在アクティブではないでの表示しない
			if (data.GetIsActive() == false) continue;

			debugRenderer->DrawSphere(data.GetPosition(), data.GetRadius(), data.GetColor());
		}
	}
}
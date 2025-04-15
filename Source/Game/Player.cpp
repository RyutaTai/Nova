#include "Player.h"

#include "../Nova/Graphics/Graphics.h"
#include "../Nova/Graphics/Camera.h"
#include "../Nova/Core/Framework.h"
#include "../Nova/Others/MathHelper.h"
#include "../Nova/Collision/Collision.h"
#include "PlayerState.h"
#include "Stage.h"
#include "EnemyManager.h"
#include "BulletManager.h"
#include "Bullet.h"

Player* Player::instance_ = nullptr;

// インスタンス取得
Player& Player::Instance()
{
	return *instance_;
}

//	コンストラクタ
Player::Player()
	:Character("./Resources/Model/Player/SKM_Manny_Anim.gltf", "")
	//:Character("./Resources/Model/Player.gltf", "")
{
	//	インスタンス設定
	_ASSERT_EXPR(instance_ == instance_, L"already instance");
	instance_ = this;

	//	ステートセット(Player::StateTypeの順と合わせる)
	stateMachine_.reset(new StateMachine<State<Player>>());
	stateMachine_->RegisterState(new PlayerState::IdleState(this));		//	待機
	stateMachine_->RegisterState(new PlayerState::MoveState(this));		//	移動
	stateMachine_->RegisterState(new PlayerState::ComboOne1(this));		//	コンボ0_1
	stateMachine_->RegisterState(new PlayerState::ComboOne2(this));		//	コンボ0_2
	stateMachine_->RegisterState(new PlayerState::ComboOne3(this));		//	コンボ0_3
	stateMachine_->RegisterState(new PlayerState::ComboOne4(this));		//	コンボ0_4
	stateMachine_->RegisterState(new PlayerState::DodgeState(this));	//	回避
	stateMachine_->RegisterState(new PlayerState::GetUpState(this));	//	起き上がり
	stateMachine_->RegisterState(new PlayerState::DamageState(this));	//	ダメージ
	stateMachine_->RegisterState(new PlayerState::FlinchState(this));	//	怯み
	stateMachine_->RegisterState(new PlayerState::DeathState(this));	//	死亡

	stateMachine_->SetState(static_cast<int>(StateType::Idle));			//	初期ステートセット
	PlayAnimation(Player::AnimationType::Idle, true, 1.0f);

	//	モデルのルート設定
	int rootNodeIndex = GetNodeIndex("root");
	SetRootJointIndex(rootNodeIndex);

	//	----- Collision -----
	RegisterCollisionData();

	//	----- オーディオ初期設定 -----
	//	リスナー情報セット
	listener_.innerRadius_ = 0.7f;
	listener_.outerRadius_ = 1.67f;
	listener_.filterParam_ = 0.8f;

	//	足音SE
	sources_[static_cast<int>(AudioStereo::Footsteps)] = AudioManager::Instance().LoadAudioSource("./Resources/Audio/SE/Player/FootstepsOne2.wav", Audio::AudioType::SENormal, "GameScene");
	sources_[static_cast<int>(AudioStereo::Footsteps)]->SetVolume(0.3f, false);
	sources_[static_cast<int>(AudioStereo::Footsteps)]->SetAudioName("PlayerFootsteps");
	AudioManager::Instance().Register(sources_[static_cast<int>(AudioStereo::Footsteps)]);

	//	攻撃ヒットSE
	sources_[static_cast<int>(AudioStereo::HitAttack)] = AudioManager::Instance().LoadAudioSource("./Resources/Audio/SE/Player/HitAttack2.wav", Audio::AudioType::SENormal, "GameScene");
	sources_[static_cast<int>(AudioStereo::HitAttack)]->SetVolume(1.0f, false);
	sources_[static_cast<int>(AudioStereo::HitAttack)]->SetAudioName("PlayerHitAttack");
	AudioManager::Instance().Register(sources_[static_cast<int>(AudioStereo::HitAttack)]);

}

//	初期化
void Player::Initialize()
{
	//	エフェクト読み込み
	effectResource_ = ResourceManager::Instance().LoadEffectResource("./Resources/Effect/HitEff.efk");

	//	エフェクトスケール設定
	effectScale_ = 0.4f;

	//	位置設定
	GetTransform()->SetPosition({ 14.0f, 0.01f, -20.0f });

	//	回転値設定
	GetTransform()->SetRotationY(DirectX::XMConvertToRadians(44.0f));

	//	スケール設定
	//GetTransform()->SetScaleFactor(3.0f);
	GetTransform()->SetScaleFactor(1.9f);

	//	座標系変換
	GetTransform()->SetCoordinateSystem(Transform::CoordinateSystem::cRightYup);

	//	当たり判定用半径、高さ設定
	radius_ = 0.7f;
	height_ = 3.4f;

	//	----- 移動速度 -----
	defaultMoveSpeed_ = 4.0f;
	moveSpeed_ = 4.0f;
	//moveSpeed_ = 25.0f;

	hp_ = MaxHp_;

	SetPixelShader("./Resources/Shader/PlayerPS.cso");

}

//	更新処理
void Player::Update(const float& elapsedTime)
{
	//	ポーズ中なら処理しない
	if (isPose_)return;

	Character::Update(elapsedTime);

	//	----- ステート更新処理 -----
	stateMachine_->Update(elapsedTime);

	//	----- 当たり判定更新 -----
	UpdateCollisionDetectionData(elapsedTime);

	//	空中にいれば
	if (isHitStage_ == false && isAddGravity_)
	{
		//	適当に重力処理
		AddVelocityY(gravity_, elapsedTime);
		//GetTransform()->SetPositionY(GetTransform()->GetPositionY() - gravity_ * elapsedTime);
		//Move(elapsedTime);	//	inputMoveにもある
	}
	//	ステージとの当たり判定
	if (isCollisionStage_)
	{
		//isHitStage_ = RayVsVertical(elapsedTime);	//	垂直方向(地面)
		if (RayVsVertical(elapsedTime) == false)
		{
			GetTransform()->AddPositionY(velocity_.y * elapsedTime);
		}
		RayVsHorizontal(elapsedTime);	//	水平方向(壁)
	}

	//Move(elapsedTime);	//	inputMoveにもある

	//	----- アニメーション更新処理 -----
	UpdateAnimation(elapsedTime);
	
	//	----- オーディオリスナー更新 -----
	UpdateListener();

}

//	リスナー情報更新
void Player::UpdateListener()
{
	//	処理中に変化する値
	DirectX::XMFLOAT3 position = GetTransform()->GetPosition();
	DirectX::XMFLOAT3 scale = GetTransform()->GetScale();

	//listener_.position = { position.x, position.y + (scale.y / 2.0f), position.z };
	listener_.position_ = { position.x, position.y + height_ / 2.0f, position.z };
	listener_.frontVec_ = Camera::Instance().GetFront();
	listener_.velocity_ = GetMoveVec();
	listener_.rightVec_ = Camera::Instance().GetRight();

}

//	当たり判定登録
void Player::RegisterCollisionData()
{
#pragma region ----- 押し出し判定登録 -----
	//	{名前、半径、  Y軸を固定するか、オフセット位置、更新名、	デフォルトカラー、	ヒットカラー}
	//	{name, radius, fixedY,			offsetPosition,	updateName,	defaultColor,		hitColor}
	
	RegisterCollisionDetectionData({ "head",						0.2f,false ,{},"" });	//	頭
	RegisterCollisionDetectionData({ "spine_04",					0.2f,false ,{},"" });	//	胸部
	RegisterCollisionDetectionData({ "upperarm_correctiveRoot_l",	0.2f,false ,{},"" });	//	左肩
	RegisterCollisionDetectionData({ "upperarm_correctiveRoot_r",	0.2f,false ,{},"" });	//	右肩
	RegisterCollisionDetectionData({ "lowerarm_l",					0.2f,false ,{},"" });	//	左肘
	RegisterCollisionDetectionData({ "lowerarm_r",					0.2f,false ,{},"" });	//	右肘
	RegisterCollisionDetectionData({ "ik_hand_l",					0.2f,false ,{},"" });	//	左手首
	RegisterCollisionDetectionData({ "ik_hand_r",					0.2f,false ,{},"" });	//	右手首
	RegisterCollisionDetectionData({ "calf_l",						0.2f,false ,{},"" });	//	左膝
	RegisterCollisionDetectionData({ "calf_r",						0.2f,false ,{},"" });	//	右膝
	RegisterCollisionDetectionData({ "ik_foot_l",					0.2f,false ,{},"" });	//	左足首
	RegisterCollisionDetectionData({ "ik_foot_r",					0.2f,false ,{},"" });	//	右足首

#pragma endregion ----- 押し出し判定登録 -----

#pragma region ----- くらい判定登録 -----
	//	{名前、半径、	オフセット位置、ダメージ倍率、	更新名、	デフォルトカラー、	ヒットカラー}
	//	{name, radius,	offsetPos,		damage,			updateName,	defaultColor,		hitColor}
	
	RegisterDamageDetectionData({ "head",						0.2f,{0.08f,0.0f,0.0f},0.3f,"" });	//	頭
	RegisterDamageDetectionData({ "spine_04",					0.2f,{},1.0f,"" });	//	胸部
	RegisterDamageDetectionData({ "upperarm_correctiveRoot_l",	0.2f,{},1.0f,"" });	//	左肩
	RegisterDamageDetectionData({ "upperarm_correctiveRoot_r",	0.2f,{},1.0f,"" });	//	右肩
	RegisterDamageDetectionData({ "lowerarm_l",					0.2f,{},1.0f,"" });	//	左肘
	RegisterDamageDetectionData({ "lowerarm_r",					0.2f,{},1.0f,"" });	//	右肘
	RegisterDamageDetectionData({ "ik_hand_l",					0.2f,{},1.0f,"" });	//	左手首
	RegisterDamageDetectionData({ "ik_hand_r",					0.2f,{},1.0f,"" });	//	右手首
	RegisterDamageDetectionData({ "calf_l",						0.2f,{},1.0f,"" });	//	左膝
	RegisterDamageDetectionData({ "calf_r",						0.2f,{},1.0f,"" });	//	右膝
	RegisterDamageDetectionData({ "ik_foot_l",					0.2f,{},1.0f,"" });	//	左足首
	RegisterDamageDetectionData({ "ik_foot_r",					0.2f,{},1.0f,"" });	//	右足首

#pragma endregion ----- くらい判定登録 -----

#pragma region ----- 攻撃判定登録 -----
	//	{名前、半径、	オフセット位置、更新名、	デフォルトカラー、	ヒットカラー}
	//	{name, radius,	offsetPos,		updateName, defaultColor,		hitColor}
	
	RegisterAttackDetectionData({ "RightPunch",	0.4f ,{},"ik_hand_r" });	//	右手のパンチ
	RegisterAttackDetectionData({ "LeftPunch",	0.4f ,{},"ik_hand_l" });	//	左手のパンチ
	RegisterAttackDetectionData({ "LeftKick",	0.4f ,{},"ik_foot_l" });	//	右のキック

	SetAllAttackDetectionActiveFlag(false);

#pragma endregion ----- 攻撃判定登録 -----

}

//	当たり判定更新
void Player::UpdateCollisionDetectionData(const float& elapsedTime)
{
	//	くらい判定更新
	for (DamageDetectionData& data : damageDetectionData_)
	{
		// ジョイントの名前で位置設定(名前がジョイントの名前ではないとき別途更新必要)
		data.SetJointPosition(GetJointPosition(data.GetUpdateName(), data.GetOffsetPosition()));

		data.Update(elapsedTime);
	}
	//	攻撃判定更新
	for (AttackDetectionData& data : attackDetectionData_)
	{
		//	ジョイントの名前で位置設定(名前がジョイントの名前ではないとき別途更新必要)
		data.SetJointPosition(GetJointPosition(data.GetUpdateName(), data.GetOffsetPosition()));
	}

	/*for (int i = AttackData::TrunAttackStart; i <= AttackData::TackleAttackEnd; ++i)
	{
		AttackDetectionData& data = GetAttackDetectionData(i);
		DirectX::XMFLOAT3 pos = data.GetPosition();
		pos.y = 1.0f;
		data.SetJointPosition(pos);
	}*/

	//	押し出し判定更新
	for (CollisionDetectionData& data : collisionDetectionData_)
	{
		//	ジョイントの名前で位置設定(名前がジョイントの名前ではないとき別途更新必要)
		DirectX::XMFLOAT3 pos = GetJointPosition(data.GetUpdateName(), data.GetOffsetPosition());

		if (data.GetFixedY())
			pos.y = 0.0f;

		data.SetPosition(pos);
		//data.SetJointPosition(pos);

		//	押し出し判定の有効フラグを更新する
		data.SetIsActive(isActiveCollisionDetection_);
	}
}

//	プレイヤーと敵の当たり判定(押し合い処理）
bool Player::PlayerVsEnemies(const float& elapsedTime)
{
	DirectX::XMFLOAT3 pos = GetTransform()->GetPosition();
	pos.y += height_ / 2.0f;
	EnemyManager& enemyManager = EnemyManager::Instance();
	DirectX::XMFLOAT3 outPosition = {};

	bool isHitEnemy = false;

	for (Enemy* enemy : enemyManager.GetEnemies())
	{
		DirectX::XMFLOAT3 ePos = enemy->GetTransform()->GetPosition();
		//float eRadius = enemy->GetRadius() - 5.0f;
		float eRadius = enemy->GetRadius();
		float eHeight = enemy->GetHeight();
		DirectX::XMFLOAT3 ePosOffset = { 0.0f,-eHeight / 2.0f,0.0f };

		//	円柱と円柱で当たり判定
		if (Collision::IntersectCylinderVsCyliner(pos, radius_, height_, ePos + ePosOffset, eRadius, eHeight, outPosition, true))
		{
			isHitEnemy = true;
			GetTransform()->SetPositionX(outPosition.x);
			GetTransform()->SetPositionZ(outPosition.z);
		}

	}
	return isHitEnemy;
}

//	ジョイントに敵または弾丸が一方でも当たっているか判定
bool Player::JointVsEnemiesAndBullet(const float& elapsedTime, const std::string& boneName, const float& jointRadius)
{
	bool isHit = false;

	//	ジョイントのワールド座標取得
	DirectX::XMFLOAT3 jointPos = GetJointPosition(boneName);
	
	//	衝突判定用のデバッグ球を描画
	DebugRenderer* debugRenderer = Graphics::Instance().GetDebugRenderer();
	//debugRenderer->DrawSphere(leftHandPos, leftHandRadius, DirectX::XMFLOAT4(1, 1, 1, 1));

	//	ジョイントと敵の当たり判定
	if (JointVsEnemies(elapsedTime, jointPos, jointRadius) == true)isHit = true;
	//	ジョイントと弾の当たり判定
	if (JointVsBullet(jointPos, jointRadius) == true)isHit = true;

	return isHit;
}

//	ジョイントと敵の当たり判定
bool Player::JointVsEnemies(const float& elapsedTime, const DirectX::XMFLOAT3& jointPos, const float& jointRadius)
{
	DirectX::XMFLOAT3 outPosition = {};
	bool isHitEnemy = false;

	for (Enemy* enemy : EnemyManager::Instance().GetEnemies())
	{
		//	敵の位置、半径、高さ
		DirectX::XMFLOAT3 ePos = enemy->GetTransform()->GetPosition();
		float eRadius = enemy->GetRadius() + 0.1f;
		float eHeight = enemy->GetHeight() * 2;
		DirectX::XMFLOAT3 ePosOffset = {};
		if (enemy->IsUseOffsetY())
		{
			ePosOffset = { 0.0f,-eHeight / 2.0f,0.0f };
		}

		//	球と円柱で当たり判定
		if (Collision::IntersectSphereVsCylinder(jointPos, jointRadius, ePos + ePosOffset, eRadius, eHeight, outPosition))
		{
			enemy->SubtractHp(1);
			float effectScale = 50.0f;
			isHitEnemy = true;
		}
		else isHitEnemy = false;

		//	エフェクト再生設定(PlayerのRender()で描画される)
		if (isHitEnemy)
		{
			isHitEnemy = true;
			SetPlayEffectFlag(true);
			SetEffectPos(jointPos);

		}
	}
	return isHitEnemy;
}

//	ジョイントと弾丸の当たり判定
bool Player::JointVsBullet(const DirectX::XMFLOAT3& jointPos, const float& jointRadius)
{
	bool isHitBullet = false;
	BulletManager& bulletManager = BulletManager::Instance();
	for (int bulletNum = 0; bulletNum < bulletManager.GetBulletCount(); ++bulletNum)
	{
		//	弾丸と右手との当たり判定
		Bullet* bullet = bulletManager.GetBullet(bulletNum);
		DirectX::XMFLOAT3	bulletPos = bullet->GetTransform()->GetPosition();	//	弾丸の位置
		float				bulletRadius = bullet->GetRadius();					//	弾丸の半径
		DirectX::XMFLOAT3	outPos = {};
		if (Collision::IntersectSphereVsSphere(jointPos, jointRadius, bulletPos, bulletRadius, outPos))
		{
			isHitBullet = true;
		}

	}
	return isHitBullet;
}

//	移動入力処理
bool Player::InputMove(const float& elapsedTime)
{
	//	進行ベクトル取得
	moveVec_ = GetMoveVec();

	//	移動処理
	UpdateVelocity(elapsedTime);
	Move(elapsedTime);

	//	旋回処理
	Turn(elapsedTime, moveVec_.x, moveVec_.z, turnSpeed_);

	//	進行ベクトルがゼロベクトルでない場合は入力された
	//	(ゼロより大きければ入力された)
	float moveVecLength = sqrtf(moveVec_.x * moveVec_.x + moveVec_.z * moveVec_.z);
	return (moveVecLength > 0);

}

////	ブレンドアニメーション
//void Player::PlayBlendAnimation(AnimationType index, bool loop, float speed)
//{
//	GameObject::PlayBlendAnimation(GetCurrentBlendAnimationIndex(), static_cast<int>(index), loop, speed);
//}

////	ステージとの当たり判定	垂直方向
//bool Player::RayVsVertical(const float& elapsedTime)
//{
//	DirectX::XMFLOAT3 rayStartPos = GetTransform()->GetPosition();	//	レイの始点
//	float centerOffset = height_ / 2.0f;							//	始点をプレイヤーの中心へ補正
//	rayStartPos.y += centerOffset;
//	DirectX::XMFLOAT3	rayDirection = { 0,-1,0 };					//	レイの方向(真下)
//	DirectX::XMFLOAT4X4 transform	= {};							//	ステージのワールド変換行列
//	DirectX::XMStoreFloat4x4(&transform, Stage::Instance().GetTransform()->CalcWorld());
//
//	float	rayLimit = centerOffset - velocity_.y * elapsedTime;	//	レイの長さ
//	//float	rayLimit = 9.0f;	//	レイの長さ
//	bool	skipIf = false;
//
//	//	レイの開始点描画
//#if 1
//	DebugRenderer* debugRenderer = Graphics::Instance().GetDebugRenderer();
//	debugRenderer->DrawSphere(rayStartPos, rayPosRadius_, DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f));	//	青
//#endif
//
//	//	レイの終点描画
//#if 1
//	DirectX::XMFLOAT3 rayEndPos = {};
//	DirectX::XMVECTOR RayEndPos = DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&rayStartPos), DirectX::XMVectorScale(DirectX::XMLoadFloat3(&rayDirection), rayLimit));
//	DirectX::XMStoreFloat3(&rayEndPos, RayEndPos);
//	debugRenderer = Graphics::Instance().GetDebugRenderer();
//	debugRenderer->DrawSphere(rayEndPos, rayPosRadius_, DirectX::XMFLOAT4(0.0f, 1.0f, 0.6f, 1.0f));		//	緑
//#endif
//
//	DirectX::XMFLOAT3	intersectionPosition	= {};			//	当たった位置
//	DirectX::XMFLOAT3	intersectionNormal		= {};			//	法線の方向
//	std::string			intersectionMesh		= {};			//	メッシュ名
//	std::string			intersectionMaterial	= {};			//	マテリアル名
//
//	//	当たり判定処理
//	bool isHit = false;
//	isHit = Stage::Instance().Collision(rayStartPos, rayDirection, transform, intersectionPosition, intersectionNormal, intersectionMesh, intersectionMaterial, rayLimit, skipIf);
//
//	//	地面にレイが当たったら
//	if (isHit)
//	{
//		hitPosition_ = intersectionPosition;
//		hitNormal_ = intersectionNormal;
//#if 0 
//		DirectX::XMFLOAT3 pos = {};
//		DirectX::XMVECTOR intersectPos = DirectX::XMLoadFloat3(&intersectionPosition);
//		DirectX::XMVECTOR pushVec = DirectX::XMVectorScale(DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&intersectionNormal)), rayLimit);
//		
//		DirectX::XMStoreFloat3(&pos, DirectX::XMVectorSubtract(intersectPos, pushVec));
//
//		GetTransform()->SetPosition(pos);
//#else
//		DirectX::XMVECTOR	IntersectionPos = DirectX::XMLoadFloat3(&intersectionPosition);			//	レイが当たった位置
//		DirectX::XMFLOAT3	pos				= GetTransform()->GetPosition();						//	プレイヤーの現在の位置(足元)
//		DirectX::XMVECTOR	Pos				= DirectX::XMLoadFloat3(&pos);							//	プレイヤーの現在の位置ベクトル
//		DirectX::XMVECTOR	Push			= DirectX::XMVectorSubtract(IntersectionPos, Pos);		//	当たった位置からプレイヤーまでのベクトル
//		
//		DirectX::XMVectorSetX(Push, 0.0f);
//		DirectX::XMVectorSetZ(Push, 0.0f);
//
//		float				pushLength		= DirectX::XMVectorGetY(DirectX::XMVector3Length(Push));
//		DirectX::XMFLOAT3	push = {};
//		DirectX::XMStoreFloat3(&push, Push);
//		if (pushLength > 0.0001f)
//		{
//			//GetTransform()->SetPosition(intersectionPosition);
//			//GetTransform()->AddPosition(push);
//			//GetTransform()->AddPositionY(push.y);
//		}
//
//		DirectX::XMFLOAT3 posDebug = GetTransform()->GetPosition();
//
//		/*DirectX::XMVECTOR Velocity = DirectX::XMLoadFloat3(&velocity_);
//		Velocity = DirectX::XMVectorAdd(Velocity, Push);
//		DirectX::XMStoreFloat3(&velocity_, Velocity);*/
//
//		velocity_.y = 0.0f;
//#endif
//
//		//	レイが当たった位置
//#if 1
//		DebugRenderer* debugRenderer = Graphics::Instance().GetDebugRenderer();
//		debugRenderer->DrawSphere(intersectionPosition, rayPosRadius_ + 1.0f, DirectX::XMFLOAT4(1, 1, 1, 1));	//	白
//#endif
//		
//	}
//
//	return isHit;
//}

//	COLLISION_MESH_2
//	ステージとの当たり判定	垂直方向
bool Player::RayVsVertical(const float& elapsedTime)
{
	DirectX::XMFLOAT3 rayStartPos;									//	レイの始点
	DirectX::XMFLOAT3 rayDirection;									//	レイの方向
	float liftup = height_ / 2.0f;									//	レイの始点をプレイヤーの中心へ持ち上げる
	DirectX::XMVECTOR RayPos = DirectX::XMLoadFloat3(&GetTransform()->GetPosition());							//	レイの始点
	DirectX::XMVECTOR Direction = DirectX::XMVector3Normalize(DirectX::XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f));	//	レイの方向
	DirectX::XMVECTOR Liftup = DirectX::XMVector3Normalize(DirectX::XMVectorSet(0.0f, liftup, 0.0f, 1.0f));		//	LIFTUP
	DirectX::XMStoreFloat3(&rayStartPos, DirectX::XMVectorAdd(RayPos, Liftup));
	DirectX::XMStoreFloat3(&rayDirection, Direction);

	DirectX::XMFLOAT3 playerPos = GetTransform()->GetPosition();	//	プレイヤーの位置(足元が基準点)

	DirectX::XMFLOAT4X4 transform = {};								//	ステージのワールド変換行列
	DirectX::XMStoreFloat4x4(&transform, Stage::Instance().GetTransform()->CalcWorld());

	//	レイの開始点描画
#if 1
	DebugRenderer* debugRenderer = Graphics::Instance().GetDebugRenderer();
	debugRenderer->DrawSphere(rayStartPos, rayPosRadius_, DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f));	//	青
#endif

	//	Collision()の結果格納用
	DirectX::XMFLOAT3	intersectionPosition = {};			//	当たった位置
	DirectX::XMFLOAT3	intersectionNormal = {};			//	法線の方向
	std::string			intersectionMesh = {};				//	メッシュ名
	std::string			intersectionMaterial = {};			//	マテリアル名

	//	当たり判定処理
	bool isHit = false;
	//	レイと地面が当たっていたら
	if (Stage::Instance().Collision(rayStartPos, rayDirection, transform, intersectionPosition, intersectionNormal, intersectionMesh, intersectionMaterial))
	{
		float d0 = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMLoadFloat3(&playerPos) - DirectX::XMLoadFloat3(&rayStartPos)));
		float d1 = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMLoadFloat3(&intersectionPosition) - DirectX::XMLoadFloat3(&rayStartPos)));

		//	プレイヤーと地面が当たっていたら
		if (d0 + radius_ > d1)
		{
			//	プレイヤーの位置を補正
			float d = d0 - d1;
			playerPos.x -= d * rayDirection.x;
			playerPos.y -= d * rayDirection.y;
			playerPos.z -= d * rayDirection.z;

			GetTransform()->SetPosition(playerPos);

			// Reflection
			//DirectX::XMStoreFloat3(&velocity_, DirectX::XMVector3Reflect(DirectX::XMLoadFloat3(&velocity_), DirectX::XMLoadFloat3(&intersectionNormal)));

			isHit = true;

			//	デバッグ描画
			//	レイが当たった位置
#if 1
			DebugRenderer* debugRenderer = Graphics::Instance().GetDebugRenderer();
			debugRenderer->DrawSphere(intersectionPosition, rayPosRadius_ + 1.0f, DirectX::XMFLOAT4(1, 1, 1, 1));	//	白
#endif

		}

	}

	return isHit;
}

//	ステージとの当たり判定	水平方向
//bool Player::RayVsHorizontal(const float& elapsedTime)
//{
//	//	水平速力計算
//	float velocityLengthXZ = sqrtf(velocity_.x * velocity_.x + velocity_.z * velocity_.z);
//	if (velocityLengthXZ > 0.0f)
//	{
//		//	水平移動値
//		float mx = velocity_.x * elapsedTime;
//		float mz = velocity_.z * elapsedTime;
//
//		//	レイの開始位置と方向
//		DirectX::XMFLOAT3	position		= GetTransform()->GetPosition();
//		DirectX::XMFLOAT3	rayStartPos			= position;
//		DirectX::XMFLOAT3	radiusOffset	= {};
//		DirectX::XMVECTOR	Move			= DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&GetMoveVec()));
//		Move = DirectX::XMVectorSetY(Move, 0.0f);
//		DirectX::XMVECTOR	RadiusOffset	= DirectX::XMVectorScale(Move, radius_);				//	移動方向の向きに半径を足す
//		DirectX::XMStoreFloat3(&radiusOffset, RadiusOffset);
//		float				heightOffset	= height_ / 2.0f;										//	レイの始点をプレイヤーの中心へ補正
//		rayStartPos.y += heightOffset;
//
//		DirectX::XMFLOAT3	rayEnd			= { rayStartPos.x + mx  ,rayStartPos.y , rayStartPos.z + mz };	//	レイの終了地点
//		DirectX::XMVECTOR	RayPos			= DirectX::XMLoadFloat3(&rayStartPos);				//	レイの開始点
//		DirectX::XMVECTOR	RayEnd			= DirectX::XMLoadFloat3(&rayEnd);
//		RayEnd = DirectX::XMVectorAdd(RayEnd, Move);
//		float				rayLimit		= DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(RayEnd, RayPos))) + radius_;
//		DirectX::XMFLOAT3	rayDirection	= {};
//		DirectX::XMVECTOR	RayDirection	= DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(RayEnd, RayPos));
//		DirectX::XMStoreFloat3(&rayDirection, RayDirection);
//		//	水平に飛ばす
//		rayDirection.y = 0;
//
//		//	ステージのワールド変換行列
//		DirectX::XMFLOAT4X4 transform = {};							
//		DirectX::XMStoreFloat4x4(&transform, Stage::Instance().GetTransform()->CalcWorld());
//
//		//	レイの開始点描画
//#if 1
//		DebugRenderer* debugRenderer = Graphics::Instance().GetDebugRenderer();
//		debugRenderer->DrawSphere(rayStartPos, rayPosRadius_, DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f));	//	青
//#endif
//
//		//	レイの終点描画
//#if 0
//		DirectX::XMFLOAT3 rayEndPos = {};
//		DirectX::XMVECTOR RayEndPos = DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&rayStartPos), DirectX::XMVectorScale(DirectX::XMLoadFloat3(&rayDirection), rayLimit));
//		DirectX::XMStoreFloat3(&rayEndPos, RayEndPos);
//		debugRenderer = Graphics::Instance().GetDebugRenderer();
//		debugRenderer->DrawSphere(rayEndPos, rayPosRadius_, DirectX::XMFLOAT4(0.0f, 1.0f, 0.6f, 1.0f));	//	緑
//#endif
//
//		//	レイキャストによる壁判定
//		DirectX::XMFLOAT3	intersectionPosition	= {};			//	当たった位置
//		DirectX::XMFLOAT3	intersectionNormal		= {};			//	法線の方向
//		std::string			intersectionMesh		= {};			//	メッシュ名
//		std::string			intersectionMaterial	= {};			//	マテリアル名
//		bool skipIf = false;
//
//		if (Stage::Instance().Collision(rayStartPos, rayDirection, transform, intersectionPosition, intersectionNormal, intersectionMesh, intersectionMaterial, rayLimit, skipIf))
//		{
//			//	壁までのベクトル
//			DirectX::XMVECTOR Start = DirectX::XMLoadFloat3(&intersectionPosition);
//			DirectX::XMVECTOR End = DirectX::XMLoadFloat3(&rayEnd);
//			DirectX::XMVECTOR Vec = DirectX::XMVectorSubtract(End, Start);
//
//			//	壁の法線
//			DirectX::XMVECTOR Normal = DirectX::XMLoadFloat3(&intersectionNormal);
//
//			//	入射ベクトルを法線に射影
//			float projectionL = DirectX::XMVectorGetX(DirectX::XMVector3Dot(DirectX::XMVectorNegate(Vec), Normal)) * 1.5f;
//
//			//	補正位置の計算
//			DirectX::XMVECTOR CollectPosition = DirectX::XMVectorAdd(
//				DirectX::XMVectorScale(Normal, projectionL), End);
//			DirectX::XMFLOAT3 collectPosition{};
//			DirectX::XMStoreFloat3(&collectPosition, CollectPosition);
//
//			//	壁ずり後の位置がめりこんでいないかチェック
//			intersectionPosition	= {};			//	当たった位置
//			intersectionNormal		= {};			//	法線の方向
//			intersectionMesh		= {};			//	メッシュ名
//			intersectionMaterial	= {};			//	マテリアル名
//			DirectX::XMStoreFloat3(&rayEnd, CollectPosition);
//			rayLimit = DirectX::XMVectorGetX(DirectX::XMVectorSubtract(RayEnd, RayPos));
//			rayDirection = {};
//			RayDirection = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(RayEnd, RayPos));
//			DirectX::XMStoreFloat3(&rayDirection, RayDirection);
//
//			if (Stage::Instance().Collision(rayStartPos, rayDirection, transform, intersectionPosition, intersectionNormal, intersectionMesh, intersectionMaterial, rayLimit, skipIf))
//			{
//				CollectPosition = DirectX::XMLoadFloat3(&intersectionPosition);
//			}
//			else
//			{
//				position.x = collectPosition.x;
//				position.z = collectPosition.z;
//			}
//
//			//	レイが当たった位置
//#if 1
//			DebugRenderer* debugRenderer = Graphics::Instance().GetDebugRenderer();
//			debugRenderer->DrawSphere(intersectionPosition, rayPosRadius_, DirectX::XMFLOAT4(1, 1, 1, 1));	//	白
//#endif
//
//		}
//		else
//		{
//			////	移動
//			//position.x += mx;
//			//position.z += mz;
//		}
//
//		GetTransform()->SetPosition(position);
//	}
//
//	return false;
//}

bool Player::RayVsHorizontal(const float& elapsedTime)
{
	DirectX::XMFLOAT3 rayStartPos;									//	レイの始点
	DirectX::XMFLOAT3 rayDirection;									//	レイの方向
	float liftup = height_ / 2.0f;									//	レイの始点をプレイヤーの中心へ持ち上げる
	DirectX::XMVECTOR RayPos = DirectX::XMLoadFloat3(&GetTransform()->GetPosition());							//	レイの始点
	DirectX::XMVECTOR Direction = DirectX::XMVector3Normalize(DirectX::XMVectorSet(velocity_.x, 0.0f, velocity_.z, 0.0f));	//	レイの方向
	DirectX::XMVECTOR Liftup = DirectX::XMVector3Normalize(DirectX::XMVectorSet(0.0f, liftup, 0.0f, 1.0f));		//	LIFTUP
#if 1
	DirectX::XMStoreFloat3(&rayStartPos, DirectX::XMVectorAdd(RayPos, Liftup));
#else
	float stepBack = 1.0f;
	DirectX::XMStoreFloat3(&rayStartPos, DirectX::XMVectorSubtract(RayPos, DirectX::XMVectorScale(Direction, stepBack)));
#endif
	DirectX::XMStoreFloat3(&rayDirection, Direction);

	DirectX::XMFLOAT3 playerPos = GetTransform()->GetPosition();	//	プレイヤーの位置(足元が基準点)

	DirectX::XMFLOAT4X4 transform = {};								//	ステージのワールド変換行列
	DirectX::XMStoreFloat4x4(&transform, Stage::Instance().GetTransform()->CalcWorld());

	//	レイの開始点描画
#if 1
	DebugRenderer* debugRenderer = Graphics::Instance().GetDebugRenderer();
	debugRenderer->DrawSphere(rayStartPos, rayPosRadius_, DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f));	//	青
#endif

	//	Collision()の結果格納用
	DirectX::XMFLOAT3	intersectionPosition = {};			//	当たった位置
	DirectX::XMFLOAT3	intersectionNormal = {};			//	法線の方向
	std::string			intersectionMesh = {};				//	メッシュ名
	std::string			intersectionMaterial = {};			//	マテリアル名

	//	当たり判定処理
	bool isHit = false;
	//	レイと地面が当たっていたら
	if (Stage::Instance().Collision(rayStartPos, rayDirection, transform, intersectionPosition, intersectionNormal, intersectionMesh, intersectionMaterial))
	{
		float d0 = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMLoadFloat3(&playerPos) - DirectX::XMLoadFloat3(&rayStartPos)));
		float d1 = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMLoadFloat3(&intersectionPosition) - DirectX::XMLoadFloat3(&rayStartPos)));

		float rayOffset = 0.5f;	//	レイの長さを少し増やす

		//	プレイヤーと地面が当たっていたら
		if (d0 + radius_ + rayOffset > d1)
		{
			//	プレイヤーの位置を補正
			float d = d0 - d1;
			playerPos.x -= d * rayDirection.x;
			playerPos.y -= d * rayDirection.y;
			playerPos.z -= d * rayDirection.z;

			GetTransform()->SetPosition(playerPos);

			// Reflection
			DirectX::XMStoreFloat3(&velocity_, DirectX::XMVector3Reflect(DirectX::XMLoadFloat3(&velocity_), DirectX::XMLoadFloat3(&intersectionNormal)));

			isHit = true;

			//	デバッグ描画
			//	レイが当たった位置
#if 1
			DebugRenderer* debugRenderer = Graphics::Instance().GetDebugRenderer();
			debugRenderer->DrawSphere(intersectionPosition, rayPosRadius_ + 1.0f, DirectX::XMFLOAT4(1, 1, 1, 1));	//	白
#endif

		}

	}

	return isHit;
}

//	エフェクト再生
void Player::PlayEffect()
{
	//	エフェクト描画フラグがfalseなら描画しない
	if (drawEffectFlag_ == false)return;

	//	エフェクト描画
	effectResource_->Play(effectPos_, effectScale_);

	//	エフェクト描画フラグリセット
	playEffectFlag_ = false;

	//	ヒット音再生
	AudioManager::Instance().GetAudioResource("PlayerHitAttack")->Play(false);

}

bool Player::DummyRay(const float& elapsedTime)
{
	//	右手のワールド座標取得
	DirectX::XMFLOAT3 leftHandPos = GetJointPosition("mixamorig:RightHandMiddle1");

	//	当たり判定用の半径セット
	constexpr float leftHandRadius = 50.0f;

	//	衝突判定用のデバッグ球を描画
	DebugRenderer* debugRenderer = Graphics::Instance().GetDebugRenderer();
	debugRenderer->DrawSphere(leftHandPos, leftHandRadius, DirectX::XMFLOAT4(1, 1, 1, 1));

	DirectX::XMFLOAT3	rayPos = leftHandPos;					//	レイの始点
	DirectX::XMFLOAT3	rayDirection = { 0,-1,0 };              //	レイの方向(真下)
	DirectX::XMFLOAT4X4 transform = {};							//	ステージのワールド変換行列
	DirectX::XMStoreFloat4x4(&transform, Stage::Instance().GetTransform()->CalcWorld());
	float	rayLimit = dummyRayLimit_;							//	レイの長さ(そのときのvelocity)
	bool	skipIf = true;										//	

	DirectX::XMFLOAT3 rayEndPos = {};
	DirectX::XMVECTOR RayEndPos = DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&rayPos), DirectX::XMVectorScale(DirectX::XMLoadFloat3(&rayDirection), rayLimit));
	DirectX::XMStoreFloat3(&rayEndPos, RayEndPos);
	isDummyHit_ = Stage::Instance().Collision(rayPos, rayDirection, transform, hitPosition_, hitNormal_, hitMaterial_, hitMesh_, rayLimit, skipIf);
	
	//	レイが当たっていなかったらヒット情報リセット
	if (isDummyReset_)
	{
		hitPosition_	= {};
		hitNormal_		= {};
		hitMesh_		= {};
		hitMaterial_	= {};
	}

	return isDummyHit_;
}

//	アニメーション
void Player::PlayAnimation(const AnimationType& animType, const bool& loop, const float& blendTime, const float& animSpeed, const float& startFrame, const float& endFrame)
{
	Character::PlayAnimation(static_cast<int>(animType), loop, blendTime, animSpeed, startFrame, endFrame);
}

//	スティック入力値から移動ベクトルを取得
DirectX::XMFLOAT3 Player::GetMoveVec()const
{
	//	入力情報を取得
	GamePad& gamePad = Input::Instance().GetGamePad();
	float ax = gamePad.GetAxisLX();
	float ay = gamePad.GetAxisLY();

	//	カメラ方向とスティックの入力値によって進行方向を計算する
	Camera& camera = Camera::Instance();
	const DirectX::XMFLOAT3& cameraRight = camera.GetRight();
	const DirectX::XMFLOAT3& cameraFoward = camera.GetFront();

	//	移動ベクトルはXZ平面に水平なベクトルになるようにする
	//	カメラ右方向ベクトルをXZ単位ベクトルに変換
	float rLength = 0.0f;
	DirectX::XMStoreFloat(&rLength, DirectX::XMVector3Length(DirectX::XMLoadFloat3(&cameraRight)));
	float cameraRightX = cameraRight.x;
	float cameraRightZ = cameraRight.z;
	float cameraRightLength = sqrtf(cameraRightX * cameraRightX + cameraRightZ * cameraRightZ);
	if (cameraRightLength > 0.0f)
	{
		//	単位ベクトル化
		cameraRightX = cameraRight.x / rLength;
		cameraRightZ = cameraRight.z / rLength;
	}

	//	カメラ前方向ベクトルをXZ単位ベクトルに変換
	float zLength = 0.0f;
	DirectX::XMStoreFloat(&zLength, DirectX::XMVector3Length(DirectX::XMLoadFloat3(&cameraFoward)));
	float cameraFrontX = cameraFoward.x;
	float cameraFrontZ = cameraFoward.z;
	float cameraFrontLength = sqrtf(cameraFrontX * cameraFrontX + cameraFrontZ * cameraFrontZ);
	if (cameraFrontLength > 0.0f)
	{
		//	単位ベクトル化
		cameraFrontX = cameraFoward.x / zLength;
		cameraFrontZ = cameraFoward.z / zLength;
	}

	//	スティックの水平入力値をカメラ右方向に反映し、
	//	スティック垂直入力値をカメラ前方向に反映し、
	//	進行ベクトルを計算する
	DirectX::XMFLOAT3 vec = {};
	vec.x = (cameraFrontX * ay + cameraRightX * ax);
	vec.z = (cameraFrontZ * ay + cameraRightZ * ax);

	//	Y軸方向には移動しない
	vec.y = 0.0f;

	return vec;
}

//	描画処理
void Player::Render()
{
	//	ピクセルシェーダーセット
	//Graphics::Instance().GetShader()->CreatePsFromCso(Graphics::Instance().GetDevice(), "./Resources/Shader/DronePS.cso", pixelShader_.ReleaseAndGetAddressOf());
	//this->SetPixelShader(pixelShader_.Get());
	Character::Render();

	//	エフェクト描画
	Graphics::Instance().GetShader()->SetDepthStencilState(Shader::DEPTH_STENCIL_STATE::ZT_OFF_ZW_OFF);
	if (playEffectFlag_)PlayEffect();

}

//	現在再生中のアニメーション番号取得
int Player::GetCurrentAnimNum()
{
	return Character::GetCurrentAnimNum();
}

//	現在再生中のアニメーションタイプ取得
Player::AnimationType Player::GetCurrentAnimType()
{
	int currentAnimNum = Character::GetCurrentAnimNum();

	return static_cast<Player::AnimationType>(currentAnimNum);
}

//	現在再生中のアニメーションの再生時間取得
const float Player::GetCurrentAnimationSeconds()
{
	return Character::GetCurrentAnimationSeconds();
}

//	アニメーションの長さ取得
const float Player::GetAnimationDuration(const AnimationType& animType)
{
	return Character::GetAnimationDuration(static_cast<int>(animType));
}

//	ステート遷移
void Player::ChangeState(const StateType& state)
{
	lastState_ = currentState_;
	currentState_ = state;
	stateMachine_->ChangeState(static_cast<int>(state));
}

//	回避ステートへ遷移
void Player::ChangeDodgeState()
{
	if (Input::Instance().GetGamePad().GetButtonDown() & GamePad::BTN_B/*Xキー*/)
	//if (Input::Instance().GetGamePad().GetButtonDown() & GamePad::BTN_Y/*Vキー*/)
	{

		ChangeState(Player::StateType::Dodge);
	}
}

//	他のステートからでも強制で遷移するステートを確認
void Player::ChangeForceExecutionState()
{
		
}

//	現在のステート表示
void Player::DrawStateStr()
{
	//	ステート文字列
	std::string stateStr[static_cast<int>(StateType::Max)] =
	{
		"Idle","Move",
		"ComboOne1","ComboOne2","ComboOne3","ComboOne4",
		"Dodge","GetUp","Damage","Flinch","Death"
	};

	ImGui::Text(u8"State　%s", stateStr[static_cast<int>(stateMachine_->GetStateIndex())].c_str());	//	ステート表示

}

//	デバッグ描画
void Player::DrawDebug()
{
	if (ImGui::TreeNode(u8"Playerプレイヤー"))
	{
		//	ステート表示
		DrawStateStr();
		stateMachine_->DrawDebug();

		//	
		Character::DrawDebug();

		//	ステージヒット文字列
		std::string hitStage = "";
		if (isHitStage_)hitStage = "true";
		else hitStage = "false";

		ImGui::Checkbox("IsPose", &isPose_);				//	ポーズフラグ
		ImGui::Checkbox("PlayEffect", &playEffectFlag_);	//	エフェクト再生フラグ
		ImGui::Checkbox("DrawEffect", &drawEffectFlag_);	//	エフェクト描画フラグ
		ImGui::Checkbox("Invincible", &isInvincible_);		//	無敵フラグ
		ImGui::Checkbox("AddGravity", &isAddGravity_);		//	重力フラグ

		//	----- コンボ -----
		ImGui::Checkbox("AutoCombo", &isAutoCombo_);		//	オートコンボフラグ
		ImGui::DragInt("ComboCount", &comboCount_);			//	コンボヒット数

		//	----- コリジョンフラグ -----
		ImGui::Checkbox("UseCollisionDetection", &isActiveCollisionDetection_);	//	押し出し判定が有効かどうか
		ImGui::Checkbox("IsCollisionSphere", &isCollisionSphere_);				//	押し出し判定
		ImGui::Checkbox("IsAttackSphere", &isAttackSphere_);					//	攻撃判定
		ImGui::Checkbox("IsDamageSphere", &isDamageSphere_);					//	くらい判定


		ImGui::DragFloat("Gravity", &gravity_, 0.1f, 0.0f);												//	重力
		ImGui::DragFloat("EffectScale", &effectScale_, 0.01f, -FLT_MAX, FLT_MAX);						//	エフェクトスケール
		//ImGui::DragFloat("AnimationSpeed", &animationSpeed_, 0.01f, -FLT_MAX, FLT_MAX);				//	アニメーション再生速度
		//ImGui::DragFloat("AnimationWeight", &weight, 0.005f, 0.0f, 1.0f);								//	アニメーションweight値
		//ImGui::DragFloat("BlendRate", &blendRate, 0.005f, 0.0f, 1.0f);								//	アニメーションブレンド率
		//ImGui::InputInt("CurrentBlendAnimationIndex", &currentAnimationIndex);						//	現在のアニメーション番号
		ImGui::Checkbox(u8"StageCollision", &isCollisionStage_);										//	ステージとの当たり判定オン/オフ
		ImGui::Text(u8"HitStage %s", hitStage.c_str());													//	ステージと当たっているか
		ImGui::DragFloat("Gravity", &gravity_, 0.01f, -FLT_MAX, FLT_MAX);								//	重力

		ImGui::DragFloat("RayPosRadius", &rayPosRadius_);	//	レイキャストの始点終点を表す球の半径

		ImGui::DragFloat3("ConeDirection", &coneDirection_.x, 0.01f, -FLT_MAX, FLT_MAX);

		DrawDummyRay();

		if (ImGui::TreeNode("3DAudio_Listener"))
		{
			//	リスナー情報
			ImGui::DragFloat3("Position", &listener_.position_.x);
			ImGui::DragFloat("InnerRadius", &listener_.innerRadius_);
			ImGui::DragFloat("OuterRadius", &listener_.outerRadius_);
			ImGui::DragFloat("FilterParam", &listener_.filterParam_);
			ImGui::DragFloat3("FrontVec", &listener_.frontVec_.x);
			ImGui::DragFloat3("RightVec", &listener_.rightVec_.x);
			ImGui::DragFloat3("Velocity", &listener_.velocity_.x);

			ImGui::TreePop();
		}

		ImGui::TreePop();
	}

	//	ImGuiでの変化を反映させる
	//SetWeight(weight);
	//SetBlendRate(blendRate);

}

//	デバッグプリミティブ描画
void Player::DrawDebugPrimitive()
{
	DebugRenderer* debugRenderer = Graphics::Instance().GetDebugRenderer();

	//	衝突判定用のデバッグ円柱を描画
	debugRenderer->DrawCylinder(this->GetTransform()->GetPosition(), radius_, height_, DirectX::XMFLOAT4(0, 0, 0, 1));

	//	円錐を描画
	debugRenderer->DrawCone(this->GetTransform()->GetPosition(), coneDirection_, radius_, height_, DirectX::XMFLOAT4{ 0,0,0,1 });

	//	----- Collision -----
	if (isCollisionSphere_)
	{
		for (auto& data : GetCollisionDetectionData())
		{
			// 現在アクティブではないため表示しない
			if (data.GetIsActive() == false) continue;

			debugRenderer->DrawSphere(data.GetPosition(), data.GetRadius(), data.GetColor());
		}
	}
	if (isDamageSphere_)
	{
		for (auto& data : GetDamageDetectionData())
		{
			debugRenderer->DrawSphere(data.GetPosition(), data.GetRadius(), data.GetColor());
		}
	}
	if (isAttackSphere_)
	{
		for (auto& data : GetAttackDetectionData())
		{
			// 現在アクティブではないため表示しない
			if (data.GetIsActive() == false) continue;

			debugRenderer->DrawSphere(data.GetPosition(), data.GetRadius(), data.GetColor());
		}
	}

}

void Player::DrawDummyRay()
{
	if (ImGui::TreeNode(u8"RayHIt"))
	{
		//ImGui::Checkbox("DummyHitReset", &isDummyReset_);
		//ImGui::Checkbox("DummyHit", &isDummyHit_);
		//ImGui::DragFloat("RayLimit", &dummyRayLimit_, 0.1f, -FLT_MAX, FLT_MAX);
		//ImGui::DragFloat("RayDebugOffset", &debugOffset_, 0.1f, -FLT_MAX, FLT_MAX);
		ImGui::DragFloat3("HitPos", &hitPosition_.x, 0.01f, -FLT_MAX, FLT_MAX);				//	当たった位置
		ImGui::DragFloat3("HitNormal", &hitNormal_.x, 0.01f, -FLT_MAX, FLT_MAX);			//	当たった面の法線
		ImGui::Text("Mesh", &hitMesh_);														//	メッシュ名
		ImGui::Text("Material", &hitMaterial_);												//	マテリアル名

		ImGui::TreePop();
	}

}
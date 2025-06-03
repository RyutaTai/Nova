#include "Player.h"

#include "../../../Nova/Graphics/Graphics.h"
#include "../../../Nova/Graphics/Camera.h"
#include "../../../Nova/Core/Framework.h"
#include "../../../Nova/Others/MathHelper.h"
#include "../../../Nova/Collision/Collision.h"
#include "../../../Nova/Resources/ResourceManager.h"
#include "../../../Game/UI/UIManager.h"
#include "../../../Game/UI/UITempo.h"
#include "../../JudgeRhythm.h"
#include "PlayerState.h"
#include "../../Stage/Stage.h"
#include "../Enemy/EnemyManager.h"
#include "../../Bullet/BulletManager.h"
#include "../../Bullet/Bullet.h"

Player* Player::instance_ = nullptr;

// インスタンス取得
Player& Player::Instance()
{
	return *instance_;
}

//	コンストラクタ
Player::Player()
	:Character("./Resources/Model/Player/Player.gltf", "")
{
	//	インスタンス設定
	_ASSERT_EXPR(instance_ == instance_, L"already instance");
	instance_ = this;

	//	----- ステートセット(Player::StateTypeの順番に合わせる) -----
	stateMachine_.reset(new StateMachine<State<Player>>());
	stateMachine_->RegisterState(new PlayerState::IdleState(this));		//	待機
	stateMachine_->RegisterState(new PlayerState::MoveState(this));		//	移動
	stateMachine_->RegisterState(new PlayerState::ComboOne1(this));		//	コンボ1_1
	stateMachine_->RegisterState(new PlayerState::ComboOne2(this));		//	コンボ1_2
	stateMachine_->RegisterState(new PlayerState::ComboOne3(this));		//	コンボ1_3
	stateMachine_->RegisterState(new PlayerState::ComboOne4(this));		//	コンボ1_4
	stateMachine_->RegisterState(new PlayerState::ComboTwo1(this));		//	コンボ2_1
	stateMachine_->RegisterState(new PlayerState::ComboTwo2(this));		//	コンボ2_2
	stateMachine_->RegisterState(new PlayerState::DodgeState(this));	//	回避
	stateMachine_->RegisterState(new PlayerState::GetUpState(this));	//	起き上がり
	stateMachine_->RegisterState(new PlayerState::DamageState(this));	//	ダメージ
	stateMachine_->RegisterState(new PlayerState::FlinchState(this));	//	怯み
	stateMachine_->RegisterState(new PlayerState::DeathState(this));	//	死亡

	stateMachine_->SetState(static_cast<int>(StateType::Idle));			//	初期ステートセット
	//	----- アニメーションセット -----
	PlayAnimation(Player::AnimationType::Idle, true, 1.0f);

	//	----- モデルのルート設定 -----
	int rootNodeIndex = GetNodeIndex("root");
	SetRootJointIndex(rootNodeIndex);

	//	----- 当たり判定登録 -----
	RegisterCollisionData();

	//	----- オーディオ初期設定 -----
	
	//	----- 足音SE -----
	sources_[static_cast<int>(AudioStereo::Footsteps)] = AudioManager::Instance().LoadAudioSource("./Resources/Audio/SE/Player/FootstepsOne2.wav", Audio::AudioType::SENormal, "GameScene");
	sources_[static_cast<int>(AudioStereo::Footsteps)]->SetVolume(0.3f, false);
	sources_[static_cast<int>(AudioStereo::Footsteps)]->SetAudioName("PlayerFootsteps");
	AudioManager::Instance().Register(sources_[static_cast<int>(AudioStereo::Footsteps)]);

	//	----- 攻撃ヒットSE -----
	sources_[static_cast<int>(AudioStereo::HitAttack)] = AudioManager::Instance().LoadAudioSource("./Resources/Audio/SE/Player/HitAttack2.wav", Audio::AudioType::SENormal, "GameScene");
	sources_[static_cast<int>(AudioStereo::HitAttack)]->SetVolume(1.0f, false);
	sources_[static_cast<int>(AudioStereo::HitAttack)]->SetAudioName("PlayerHitAttack");
	AudioManager::Instance().Register(sources_[static_cast<int>(AudioStereo::HitAttack)]);

}

//	初期化
void Player::Initialize()
{
	//	----- エフェクト読み込み -----
	effectResource_ = ResourceManager::Instance().LoadEffectResource("./Resources/Effect/HitEff.efk");

	//	----- エフェクトスケール設定 -----
	effectScale_ = 0.4f;

	//	----- 位置設定 -----
	GetTransform()->SetPosition({ 14.0f, 0.01f, -20.0f });

	//	----- 回転値設定 -----
	GetTransform()->SetRotationY(DirectX::XMConvertToRadians(44.0f));

	//	----- スケール設定 -----
	GetTransform()->SetScaleFactor(1.9f);

	//	----- 座標系変換 -----
	GetTransform()->SetCoordinateSystem(Transform::CoordinateSystem::cRightYup);

	//	----- 当たり判定用半径、高さ設定 -----
	radius_ = 0.7f;
	height_ = 3.4f;

	//	----- 移動速度 -----
	defaultMoveSpeed_ = 4.0f;
	moveSpeed_ = 4.0f;

	//	----- HP設定 -----
	hp_ = MaxHp_;

	//	----- ピクセルシェーダーセット -----
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
		//	重力処理
		AddVelocityY(gravity_, elapsedTime);
		//GetTransform()->SetPositionY(GetTransform()->GetPositionY() - gravity_ * elapsedTime);
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

	//	----- アニメーション更新処理 -----
	UpdateAnimation(elapsedTime);
	
	
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
	//	----- くらい判定更新 -----
	for (DamageDetectionData& data : damageDetectionData_)
	{
		// ジョイントの名前で位置設定(名前がジョイントの名前ではないとき別途更新必要)
		data.SetJointPosition(GetJointPosition(data.GetUpdateName(), data.GetOffsetPosition()));

		data.Update(elapsedTime);
	}
	//	----- 攻撃判定更新 -----
	for (AttackDetectionData& data : attackDetectionData_)
	{
		//	ジョイントの名前で位置設定(名前がジョイントの名前ではないとき別途更新必要)
		data.SetJointPosition(GetJointPosition(data.GetUpdateName(), data.GetOffsetPosition()));
	}

	//	----- 押し出し判定更新 -----
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

//	移動入力処理
bool Player::InputMove(const float& elapsedTime)
{
	//	----- 移動処理 -----
	UpdateVelocity(elapsedTime);
	Move(elapsedTime);

	//	----- 進行方向更新 -----
	moveVec_ = GetMoveVec();

	//	----- 旋回処理 -----
	Turn(elapsedTime, moveVec_.x, moveVec_.z, turnSpeed_);

	//	進行ベクトルがゼロベクトルでない場合は入力された
	//	(ゼロより大きければ入力された)
	float moveVecLength = sqrtf(moveVec_.x * moveVec_.x + moveVec_.z * moveVec_.z);
	return (moveVecLength > 0);

}

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

			//	当たり判定フラグを立てる
			isHit = true;

			//	デバッグ描画
			//	レイが当たった位置
#if 1
			DebugRenderer* debugRenderer = Graphics::Instance().GetDebugRenderer();
			debugRenderer->DrawSphere(intersectionPosition, rayPosRadius_ , DirectX::XMFLOAT4(1, 1, 1, 1));	//	白
#endif

		}

	}

	return isHit;
}

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

	//	当たり判定結果格納用
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
			//DirectX::XMStoreFloat3(&velocity_, DirectX::XMVector3Reflect(DirectX::XMLoadFloat3(&velocity_), DirectX::XMLoadFloat3(&intersectionNormal)));

			//	当たり判定フラグを立てる
			isHit = true;

			//	デバッグ描画
			//	レイが当たった位置
#if 1
			DebugRenderer* debugRenderer = Graphics::Instance().GetDebugRenderer();
			debugRenderer->DrawSphere(intersectionPosition, rayPosRadius_ , DirectX::XMFLOAT4(1, 1, 1, 1));	//	白
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
	{
		JudgeRhythm::Instance().Judge();
		ChangeState(Player::StateType::Dodge);
	}
}

//	現在のステート表示
void Player::DrawStateStr()
{
	//	ステート文字列
	std::string stateStr[static_cast<int>(StateType::Max)] =
	{
		"Idle","Move",
		"ComboOne1","ComboOne2","ComboOne3","ComboOne4",
		"ComboTwo1","ComboTwo2",
		"Dodge","GetUp","Damage","Flinch","Death"
	};

	ImGui::Text(u8"State　%s", stateStr[static_cast<int>(stateMachine_->GetStateIndex())].c_str());	//	ステート表示

}

//	デバッグ描画
void Player::DrawDebug()
{
	if (ImGui::TreeNode(u8"Playerプレイヤー"))
	{
		//	----- ステート -----
		DrawStateStr();				//	現在のステート表示
		stateMachine_->DrawDebug();	//	各ステートのデバッグ表示

		//	----- キャラクター共通のデバッグ表示 -----
		Character::DrawDebug();

		//	----- レイキャスト -----
		std::string hitStage = "";
		if (isHitStage_)hitStage = "true";
		else hitStage = "false";
		ImGui::DragFloat("RayPosRadius", &rayPosRadius_);	//	レイキャストの始点終点を表す球の半径
		ImGui::Checkbox(u8"StageCollision", &isCollisionStage_);										//	ステージとの当たり判定オン/オフ
		ImGui::Text(u8"HitStage %s", hitStage.c_str());													//	ステージと当たっているか
		
		//	----- 重力 -----
		ImGui::DragFloat("Gravity", &gravity_, 0.01f, -FLT_MAX, FLT_MAX);								//	重力
		
		//	----- ポーズフラグ -----
		ImGui::Checkbox("IsPose", &isPose_);

		//	----- コンボ -----
		ImGui::Checkbox("AutoCombo", &isAutoCombo_);		//	オートコンボフラグ
		ImGui::DragInt("ComboCount", &comboCount_);			//	コンボヒット数

		//	----- コリジョンフラグ -----
		ImGui::Checkbox("UseCollisionDetection", &isActiveCollisionDetection_);	//	押し出し判定が有効かどうか
		ImGui::Checkbox("IsCollisionSphere", &isCollisionSphere_);				//	押し出し判定
		ImGui::Checkbox("IsAttackSphere", &isAttackSphere_);					//	攻撃判定
		ImGui::Checkbox("IsDamageSphere", &isDamageSphere_);					//	くらい判定

		//	----- エフェクト -----
		ImGui::DragFloat("EffectScale", &effectScale_, 0.01f, -FLT_MAX, FLT_MAX);						//	エフェクトスケール
		ImGui::Checkbox("PlayEffect", &playEffectFlag_);	//	エフェクト再生フラグ
		ImGui::Checkbox("DrawEffect", &drawEffectFlag_);	//	エフェクト描画フラグ
		ImGui::Checkbox("AddGravity", &isAddGravity_);		//	重力フラグ

		
		ImGui::TreePop();
	}
}

//	デバッグプリミティブ描画
void Player::DrawDebugPrimitive()
{
	DebugRenderer* debugRenderer = Graphics::Instance().GetDebugRenderer();

	//	衝突判定用のデバッグ円柱を描画
	debugRenderer->DrawCylinder(this->GetTransform()->GetPosition(), radius_, height_, DirectX::XMFLOAT4(0, 0, 0, 1));

	//	----- Collision -----
	if (isCollisionSphere_)
	{
		for (auto& data : GetCollisionDetectionData())
		{
			//	現在アクティブではないため表示しない
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
			//	現在アクティブではないため表示しない
			if (data.GetIsActive() == false) continue;

			debugRenderer->DrawSphere(data.GetPosition(), data.GetRadius(), data.GetColor());
		}
	}

}

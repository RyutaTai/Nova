#include "Drone.h"

#include "DroneState.h"
#include "../../../../Nova/Graphics/Graphics.h"
#include "../../../../Nova/Input/Input.h"
#include "../../../../Nova/Core/Framework.h"
#include "../../../../Nova/Others/MathHelper.h"
#include "../../../../Nova/Others/Converter.h"
#include "../../../../Nova/Resources/ResourceManager.h"
#include "../../../../Nova/Camera/Camera.h"
#include "../../../Bullet/BulletStraight.h"
#include "../../../Bullet/BulletHorming.h"
#include "../../Player/Player.h"
#include "../../../Stage/Stage.h"

Drone::Drone()
	:Enemy("./Resources/Model/Drone/Drone.gltf")
{
	//	----- 自分の種類を設定 -----
	myType_ = EnemyType::Drone;

	//	----- ステートセット(Drone::StateTypeの順と合わせる) -----
	stateMachine_.reset(new StateMachine<State<Drone>>());
	stateMachine_->RegisterState(new DroneState::IdleState(this));			//	待機
	stateMachine_->RegisterState(new DroneState::SearchState(this));		//	探索
	stateMachine_->RegisterState(new DroneState::MoveState(this));			//	移動
	stateMachine_->RegisterState(new DroneState::PursuitState(this));		//	追跡
	stateMachine_->RegisterState(new DroneState::AttackState(this));		//	攻撃
	stateMachine_->RegisterState(new DroneState::AvoidanceState(this));		//	回避
	stateMachine_->RegisterState(new DroneState::DamageState(this));		//	ダメージ
	stateMachine_->RegisterState(new DroneState::DeathState(this));			//	死亡

	//	----- 初期ステート設定 -----
	stateMachine_->SetState(static_cast<int>(StateType::Idle));

}

//	デストラクタ
Drone::~Drone()
{
	
}

//	初期化
void Drone::Initialize()
{
	//	----- 自身の種類を設定 -----
	myType_ = EnemyType::Drone;

	//	----- 角度設定 -----
	float angleY = ConvertToRadian(220.0f);
	GetTransform()->SetRotationY(angleY);

	//	----- スケール -----
	float scale = 0.6f;
	GetTransform()->SetScaleFactor(scale);

	//	----- 当たり判定データ登録 -----
	RegisterCollisionData("./Resources/Json/DroneCollisionData.json");

	//	----- 半径、高さ設定 -----
	height_ = 1.0f;
	radius_ = 2.5f;

	//	----- 索敵範囲設定 -----
	searchRange_ = 15.0f;

	//	----- HP設定 -----
	hp_ = MaxHp_;

	//	----- 移動速度 -----
	moveSpeed_ = 6.5f;

	//	----- 弾丸初期化 -----
	BulletManager::Instance().Initialize();

	//	----- エフェクト設定 -----
	effectResource_ = ResourceManager::Instance().LoadEffectResource("./Resources/Effect/HitEff.efk");
	effectScale_ = 0.8f;

	/* ----- オーディオ初期化 ----- */
#if 1
	DirectX::XMFLOAT3 playerPos = Player::Instance().GetTransform()->GetPosition();
	float playerHeight = Player::Instance().GetHeight();
	float posOffsetY = -10.0f;

	//	エミッターの設定
	emitter_ =	std::make_shared<SoundEmitter>();
	emitter_->position_ = GetTransform()->GetPosition();
	//emitter_[static_cast<int>(Audio3D::Shot)].position.y = playerPos.y + playerHeight / 2.0f + posOffsetY;
	emitter_->velocity_ = { 1.0f, 2.0f, 1.0f };
	emitter_->minDistance_ = 7.0f;
	emitter_->maxDistance_ = 22.0f;
	emitter_->volume_ = 1.0f;
	emitter_->name_ = "Drone";
	AudioManager::Instance().EmitterRegister(emitter_);

	//	発射音
	shotSE_ = AudioManager::Instance().LoadAudioSource3D("./Resources/Audio/SE/Drone/launchSE.wav", Audio::AudioType::SE3D, "GameScene", emitter_);
	shotSE_->SetVolume(0.3f, false);
	shotSE_->SetAudioName("LaunchBullet");
	shotSE_->SetDSPSetting(Camera::Instance().GetListener());
	shotSE_->SetListenerName(Camera::Instance().GetListener()->name_);
	AudioManager::Instance().AudioRegister(shotSE_);

	std::shared_ptr<AudioSource3D> bgm3D = AudioManager::Instance().LoadAudioSource3D("./Resources/Audio/BGM/Game.wav", Audio::AudioType::BGM3D, "GameScene", emitter_);
	bgm3D->SetVolume(0.3f, false);
	bgm3D->SetAudioName("GameBGM3D");
	bgm3D->SetDSPSetting(Camera::Instance().GetListener());
	bgm3D->SetListenerName(Camera::Instance().GetListener()->name_);
	bgm3D->Play(true);
	AudioManager::Instance().AudioRegister(bgm3D);
#endif

}

//	更新処理
void Drone::Update(const float& elapsedTime)
{
	Character::Update(elapsedTime);

	//	----- ターゲット位置更新 -----
	UpdateTargetPosition();

	//	----- ステート更新処理 -----
	stateMachine_->Update(elapsedTime);

	//	----- 移動更新 -----
	UpdateVelocity(elapsedTime);

	//	----- 当たり判定更新 -----
	UpdateCollisions(elapsedTime);

	//	----- ステージとの当たり判定 -----
	isHitStage_ = RayVsHorizontal(elapsedTime);

	//	----- 位置更新 -----
	UpdatePosition(elapsedTime);

	//	----- 旋回処理 -----
	Turn(elapsedTime);

	//	----- 発射タイマー更新 -----
	//UpdateLaunchTimer(elapsedTime);

	//	----- 弾丸更新処理 -----
 	BulletManager::Instance().Update(elapsedTime);
	BulletManager::Instance().CoverModelUpdate(elapsedTime);

	//	----- オーディオ更新 -----
	UpdateEmitter();
	UpdateAudioSource();
	
}

//	ダメージステートへ遷移
void Drone::ChangeDamageState()
{
	if (isDamaged_)ChangeState(StateType::Damage);
}

//	死亡判定
void Drone::JudgeDeath()
{
	//	死亡フラグがtrueなら死亡ステートへ遷移
	if (isDead_)
	{
		ChangeState(StateType::Death);
	}
}

//	死んだときに一回呼ばれる
void Drone::OnDead()
{
	JudgeDeath();
}

//	エミッター更新
void Drone::UpdateEmitter()
{
	emitter_->position_ = GetTransform()->GetPosition();
}

//	オーディオソース更新
void Drone::UpdateAudioSource()
{
	//	発射音
	shotSE_->SetEmitterPosition(emitter_->position_);
	shotSE_->SetDSPSetting(Camera::Instance().GetListener());
		
}

//	弾丸処理
void Drone::LaunchBullet(const float& elapsedTime)
{
	//	弾丸発射フラグが立っていなければreturn
	if (isBulletLaunch_ == false)return;

#if 1
	//	----- 発射タイマー更新 -----
	UpdateLaunchTimer(elapsedTime);

	//	----- 一定間隔で弾を発射 -----
	if (launchTimer_ >= launchInterval_)
#else
	GamePad gamePad = Input::Instance().GetGamePad();
	if (gamePad.GetButtonDown() & GamePad::BTN_START)	//	Enterキーで発射
#endif
	{
		//	発射音再生
		AudioManager::Instance().GetAudioResource("LaunchBullet")->Play(false);

		//	前方向
		DirectX::XMFLOAT3 dir = {};
		float angleY = GetTransform()->GetRotationY();

		dir.x = sinf(angleY);
		dir.y = 0.0f;
		dir.z = cosf(angleY);

		//	発射位置
		DirectX::XMFLOAT3 pos = GetTransform()->GetPosition();
		pos = pos + dir * 2.0f;

		//	弾丸モデルのファイル名
#if  1	//	直進する弾丸生成
		BulletStraight* bullet = new BulletStraight();
		bullet->Launch(dir, pos);

#else	//	追従する弾丸生成
		BulletHorming* bullet = new BulletHorming();
		bullet->Launch(dir, pos);
#endif	
		//	所有者の位置設定
		bullet->SetOwnerPosition(GetTransform()->GetPosition());

		//	発射タイマーリセット
		ResetLaunchTimer();

	}

}

//	ステージとの当たり判定
bool Drone::RayVsVertical(const float& elapsedTime)
{

	return false;
}

bool Drone::RayVsHorizontal(const float& elapsedTime)
{
	DirectX::XMFLOAT3 rayStartPos;									//	レイの始点
	DirectX::XMFLOAT3 rayDirection;									//	レイの方向
	float liftup = height_ / 2.0f;									//	レイの始点をドローンの中心へ持ち上げる
	DirectX::XMVECTOR RayPos = DirectX::XMLoadFloat3(&GetTransform()->GetPosition());							//	レイの始点
	DirectX::XMVECTOR Direction = DirectX::XMVector3Normalize(DirectX::XMVectorSet(velocity_.x, 0.0f, velocity_.z, 0.0f));	//	レイの方向
	DirectX::XMVECTOR Liftup = DirectX::XMVector3Normalize(DirectX::XMVectorSet(0.0f, liftup, 0.0f, 1.0f));		//	LIFTUP
#if 1
	DirectX::XMStoreFloat3(&rayStartPos, DirectX::XMVectorAdd(Liftup, RayPos));
#else
	float stepBack = 1.0f;
	DirectX::XMStoreFloat3(&rayStartPos, DirectX::XMVectorSubtract(RayPos, DirectX::XMVectorScale(Direction, stepBack)));
#endif
	DirectX::XMStoreFloat3(&rayDirection, Direction);

	DirectX::XMFLOAT3 myPosition = GetTransform()->GetPosition();	//	ドローンの位置

	DirectX::XMFLOAT4X4 transform = {};								//	ステージのワールド変換行列
	DirectX::XMStoreFloat4x4(&transform, Stage::Instance().GetTransform()->CalcWorld());

	//	当たり判定結果格納用
	DirectX::XMFLOAT3	intersectionPosition = {};			//	当たった位置
	DirectX::XMFLOAT3	intersectionNormal = {};			//	法線の方向
	std::string			intersectionMesh = {};				//	メッシュ名
	std::string			intersectionMaterial = {};			//	マテリアル名

	//	当たり判定処理
	bool isHit = false;
	//	レイが当たっていたら
	if (Stage::Instance().Collision(rayStartPos, rayDirection, transform, intersectionPosition, intersectionNormal, intersectionMesh, intersectionMaterial))
	{
		float d0 = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMLoadFloat3(&myPosition) - DirectX::XMLoadFloat3(&rayStartPos)));
		float d1 = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMLoadFloat3(&intersectionPosition) - DirectX::XMLoadFloat3(&rayStartPos)));

		float rayOffset = 0.5f;	//	レイの長さを少し増やす

		//	ドローンとステージが当たっていたら
		if (d0 + radius_ + rayOffset > d1)
		{
			//	ドローンの位置を補正
			float d = d0 - d1;
			myPosition.x -= d * rayDirection.x;
			myPosition.y -= d * rayDirection.y;
			myPosition.z -= d * rayDirection.z;

			GetTransform()->SetPosition(myPosition);
			velocity_ = {};

			// Reflection
			//DirectX::XMStoreFloat3(&velocity_, DirectX::XMVector3Reflect(DirectX::XMLoadFloat3(&velocity_), DirectX::XMLoadFloat3(&intersectionNormal)));

			//	当たり判定フラグを立てる
			isHit = true;

		}
	}

	return isHit;
}

//	破棄処理
void Drone::Destroy()
{
	//	----- エフェクト再生 -----
	DirectX::XMFLOAT3 effectPos = GetTransform()->GetPosition();
	effectResource_->Play(effectPos, effectScale_);
	
	Enemy::Destroy();	//	自身を破棄
}

//	当たり判定登録
void Drone::RegisterCollisionData(const std::string& jsonFileName)
{
	//	Json書き出しパスとファイル名を設定
	collisionDataJsonFileName_ = jsonFileName;

	//	Jsonから読み込んで当たり判定データを設定
	LoadCollisionDataFromJson(collisionDataJsonFileName_);

}

//	当たり判定更新
void Drone::UpdateCollisions(const float& elapsedTime)
{
	//	くらい判定更新
	for (DamageDetectionData& data : damageDetectionData_)
	{
		//	ジョイントの名前で位置設定(名前がジョイントの名前ではないとき別途更新必要)
		data.SetJointPosition(GetJointPosition(data.GetUpdateName(), data.GetOffsetPosition()));

		data.Update(elapsedTime);
	}

	//	攻撃判定更新
	for (AttackDetectionData& data : attackDetectionData_)
	{
		//	ジョイントの名前で位置設定(名前がジョイントの名前ではないとき別途更新必要)
		data.SetJointPosition(GetJointPosition(data.GetUpdateName(), data.GetOffsetPosition()));
	}

	//	押し出し判定更新
	for (CollisionDetectionData& data : collisionDetectionData_)
	{
		//	ジョイントの名前で位置設定(名前がジョイントの名前ではないとき別途更新必要)
		DirectX::XMFLOAT3 pos = GetJointPosition(data.GetUpdateName(), data.GetOffsetPosition());

		//	Y軸固定
		if (data.GetFixedY())
			pos.y = 0.0f;

		data.SetPosition(pos);
	}
}

//	描画処理
void Drone::Render()
{
	//	ステート設定
	Graphics::Instance().GetShader()->SetRasterizerState(Shader::RASTERIZER_STATE::CULL_NONE);
	Graphics::Instance().GetShader()->SetDepthStencilState(Shader::DEPTH_STENCIL_STATE::ZT_ON_ZW_ON);
	Graphics::Instance().GetShader()->SetBlendState(Shader::BLEND_STATE::ALPHA);

	//	モデル描画
	Character::Render();

	//	弾丸描画
	BulletManager::Instance().Render();

}

//	デバッグプリミティブ描画
void Drone::DrawDebugPrimitive()
{
	DebugRenderer* debugRenderer = Graphics::Instance().GetDebugRenderer();

	//	衝突判定用のデバッグ球を描画
	debugRenderer->DrawCylinder(GetTransform()->GetPosition(), radius_, height_, DirectX::XMFLOAT4(0, 0, 0, 1));

	//	索敵範囲描画(円柱)
	debugRenderer->DrawCylinder(GetTransform()->GetPosition(), searchRange_, 1.0f, { 0.0f,1.0f,0.1f,1.0f });
	//	射程範囲描画(円柱)
	debugRenderer->DrawCylinder(GetTransform()->GetPosition(), launchRange_, 1.0f, { 1.0f,0.1f,0.1f,1.0f });
	
	//	弾丸のデバッグ球描画
	BulletManager::Instance().DrawDebugPrimitive();

	//	当たり判定表示
	Character::DrawDebugPrimitive();

}

//	現在のステート表示
void Drone::DrawStateStr()
{
	//	ステート文字列
	std::string stateStr[static_cast<int>(StateType::Max)] =
	{
		"Idle","Search","Move","Pursuit",
		"Attack","Avoidance","Damage","Death",
	};

	ImGui::Text(u8"State　%s", stateStr[static_cast<int>(stateMachine_->GetCurrentStateIndex())].c_str());	//	ステート表示

}

//	デバッグ描画
void Drone::DrawDebug()
{
	if (ImGui::TreeNode(u8"Drone ドローン"))
	{
		//	----- ステート -----
		DrawStateStr();
		stateMachine_->DrawDebug();

		Character::DrawDebug();

		//	----- ステージに当たっているか -----
		ImGui::Checkbox("IsHitStage", &isHitStage_);

		//	----- ターゲット -----
		ImGui::Text("----- Target -----");
		ImGui::DragFloat3("TargetPos", &targetPosition_.x);
		float distanceToTarget = CalcDistanceToTarget();
		ImGui::DragFloat("DistanceToTarget", &distanceToTarget, 0.1f, -FLT_MAX, FLT_MAX);	//	ターゲットまでの距離
		ImGui::DragFloat("SerchRange", &searchRange_, 0.1f, -FLT_MAX, FLT_MAX);				//	索敵範囲

		//	----- 弾丸 -----
		ImGui::Text("----- Bullet -----");
		ImGui::Checkbox("Bullet Launch ", &isBulletLaunch_);						//	弾丸発射処理をするかどうか
		ImGui::DragFloat("LaunchTimer", &launchTimer_, 0.01f);						//	発射タイマー
		ImGui::DragFloat("LaunchInterval", &launchInterval_, 0.01f);				//	発射間隔
		ImGui::DragFloat("LaunchRange", &launchRange_, 0.1f, -FLT_MAX, FLT_MAX);	//	射程範囲
		BulletManager::Instance().DrawDebug();	//	弾丸ImGui

		ImGui::TreePop();
	}

}
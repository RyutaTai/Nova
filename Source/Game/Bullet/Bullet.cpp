#include "Bullet.h"

#include "../../Nova/Debug/DebugRenderer.h"
#include "../../Nova/Graphics/Graphics.h"
#include "../../Nova/Collision/Collision.h"
#include "../../Nova/Audio/AudioManager.h"
#include "../../Nova/Resources/ResourceManager.h"
#include "../Character/Player/Player.h"
#include "../../Nova/Camera/Camera.h"
#include "../Stage/Stage.h"

Bullet::Bullet()
{
	//	----- モデル生成 -----
	gltfStaticModelResource_ = ResourceManager::Instance().LoadGltfModelStaticResource("./Resources/Model/Bullet/Sphere.gltf");

	//	----- 生成時にマネージャーに登録する -----
	BulletManager::Instance().Register(this);

	//	----- 弾丸半径(当たり判定用) -----
	radius_ = 0.5f;

	//	----- スケール -----
	GetTransform()->SetScaleFactor(0.4f);
	
	//	----- 攻撃相手を設定 -----
	opponentType_ = OpponentType::Player;

	//	----- 生存時間 -----
	lifeTimer_ = 2.5f;

	//	----- オーディオ -----
	emitter_ = std::make_shared<SoundEmitter>();
	emitter_->position_ = GetTransform()->GetPosition();
	//emitter_.velocity_ = velocity_;
	emitter_->velocity_ = { 1,2,1 };
	emitter_->minDistance_ = 7.0f;
	emitter_->maxDistance_ = 22.0f;
	emitter_->volume_ = 1.0f;
	emitter_->name_ = "Bullet";
	AudioManager::Instance().EmitterRegister(emitter_);

	//	移動SE
	moveSE_ = AudioManager::Instance().LoadAudioSource3D("./Resources/Audio/SE/Bullet/bulletMove.wav", Audio::AudioType::SE3D, "GameScene", emitter_.get());
	moveSE_->SetVolume(0.5f, false);
	moveSE_->SetAudioName("BulletMove");
	moveSE_->SetDSPSetting(Camera::Instance().GetListener());
	moveSE_->SetListenerName(Camera::Instance().GetListener()->name_);
	AudioManager::Instance().AudioRegister(moveSE_);

	//	----- エフェクト -----
	effectResource_[static_cast<int>(EffectType::Explosion)] = ResourceManager::Instance().LoadEffectResource("./Resources/Effect/Blow11_2.efk");
	effectScale_[static_cast<int>(EffectType::Explosion)] = 0.3f;

	//	----- カバーモデル読み込み -----
	DirectX::XMFLOAT4 coverModelColor = { 1.0f,0.0f,0.0f,1.0f };
	coverModel_ = std::make_unique<GltfModelStaticBatching>("./Resources/Model/Cube/Cube.gltf", true, coverModelColor);
	//	----- スケール設定 -----
	coverModel_->GetTransform()->SetScaleFactor(0.4f);

	//	----- ピクセルシェーダーセット -----
	coverModel_->SetPixelShader("./Resources/Shader/BulletCoverPS.cso");

	//	----- 移動速度設定 -----
	moveSpeed_ = 6.0f;

}

Bullet::~Bullet()
{

}

//	初期化処理
void Bullet::Initialize()
{

}

//	更新処理
void Bullet::Update(const float& elapsedTime)
{
	//	更新フラグがfalseなら処理しない
	if (updateFlag_ == false)return;

	//	----- ステージとの当たり判定 -----
	RayVsHorizontal(elapsedTime);

	//	----- 位置更新 -----
	UpdatePosition();

	//	----- 生存時間更新 -----
	UpdateLifeTimer(elapsedTime);

	//	----- オーディオ関連更新 -----
	UpdateEmitter();
	UpdateAudioSource();

}

//	位置更新
void Bullet::UpdatePosition()
{
	GetTransform()->AddPosition(velocity_);
	velocity_ = {};
}

//	発射
void Bullet::Launch(const DirectX::XMFLOAT3& direction, const DirectX::XMFLOAT3& position)
{
	//	移動音再生
	AudioManager::Instance().GetAudioResource("BulletMove")->Play(false);
	
}

//	エミッター更新
void Bullet::UpdateEmitter()
{
	//	位置更新
	//emitter_.position_ = GetTransform()->GetPosition();
	//se_[static_cast<int>(Audio3D::Move)]->SetPosition(emitter_.position_);	//	AudioSource3Dのemitter_のpositionに渡す
	////emitter_.velocity_ = velocity_;
	
	moveSE_->SetEmitterPosition(GetTransform()->GetPosition());	//	AudioSource3Dのemitter_のpositionに渡す
	//emitter_.velocity_ = velocity_;
}

//	オーディオソース更新
void Bullet::UpdateAudioSource()
{
	if (moveSE_)
	{
		moveSE_->SetDSPSetting(Camera::Instance().GetListener());
	}
}

//	カバーモデル更新処理
void Bullet::CoverModelUpdate(const float& elpasedTime)
{
	//	位置更新
	DirectX::XMFLOAT3 bulletPos = this->GetTransform()->GetPosition();
	coverModel_->GetTransform()->SetPosition(bulletPos);
}

//	ステージとの当たり判定(水平方向)
bool Bullet::RayVsHorizontal(const float& elapsedTime)
{
	DirectX::XMFLOAT3 rayStartPos;									//	レイの始点
	DirectX::XMFLOAT3 rayDirection;									//	レイの方向
	DirectX::XMVECTOR RayPos = DirectX::XMLoadFloat3(&GetTransform()->GetPosition());							//	レイの始点
	DirectX::XMVECTOR Direction = DirectX::XMVector3Normalize(DirectX::XMVectorSet(velocity_.x, 0.0f, velocity_.z, 0.0f));	//	レイの方向

	DirectX::XMStoreFloat3(&rayDirection, Direction);

	DirectX::XMFLOAT3 myPosition = GetTransform()->GetPosition();	//	弾丸の位置

	DirectX::XMFLOAT4X4 transform = {};								//	ステージのワールド変換行列
	DirectX::XMStoreFloat4x4(&transform, Stage::Instance().GetTransform()->CalcWorld());

	//	当たり判定結果格納用
	DirectX::XMFLOAT3	intersectionPosition = {};			//	当たった位置
	DirectX::XMFLOAT3	intersectionNormal = {};			//	法線の方向
	std::string			intersectionMesh = {};				//	メッシュ名
	std::string			intersectionMaterial = {};			//	マテリアル名

	//	当たり判定処理
	bool isHit = false;
	//	レイと地面が当たっていたら
	DirectX::XMStoreFloat3(&rayStartPos, RayPos);
	if (Stage::Instance().Collision(rayStartPos, rayDirection, transform, intersectionPosition, intersectionNormal, intersectionMesh, intersectionMaterial))
	{
		float d0 = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMLoadFloat3(&myPosition) - DirectX::XMLoadFloat3(&rayStartPos)));
		float d1 = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMLoadFloat3(&intersectionPosition) - DirectX::XMLoadFloat3(&rayStartPos)));

		float rayOffset = 0.5f;	//	レイの長さを少し増やす

		//	プレイヤーと地面が当たっていたら
		if (d0 + radius_ + rayOffset > d1)
		{
			//	プレイヤーの位置を補正
			float d = d0 - d1;
			myPosition.x -= d * rayDirection.x;
			myPosition.y -= d * rayDirection.y;
			myPosition.z -= d * rayDirection.z;

			Destroy();

			// Reflection
			//DirectX::XMStoreFloat3(&velocity_, DirectX::XMVector3Reflect(DirectX::XMLoadFloat3(&velocity_), DirectX::XMLoadFloat3(&intersectionNormal)));

			//	当たり判定フラグを立てる
			isHit = true;
		}
	}

	return isHit;
}

//	破棄
void Bullet::Destroy()
{
	//	無敵状態なら破棄しない
	if (isInvincible_)return;

	//	エフェクト描画
	effectResource_[static_cast<int>(EffectType::Explosion)]->Play(GetTransform()->GetPosition(), effectScale_[static_cast<int>(EffectType::Explosion)]);

	//	オーディオ削除
	AudioManager::Instance().GetAudioResource("BulletMove")->Stop();

	//	マネージャーから自分を削除する
	BulletManager::Instance().Remove(this);

	//	更新フラグをオフにする
	updateFlag_ = false;

}

//	生存時間更新
void Bullet::UpdateLifeTimer(const float& elapsedTime)
{
	lifeTimer_ -= elapsedTime;

	//	生存時間が無くなったら破棄
	if (lifeTimer_ <= 0.0f)
	{
		Destroy();
	}

}

//	シャドウマップ
void Bullet::CastShadows()
{
	gltfStaticModelResource_->CastShadows();
	coverModel_->CastShadows();
}

//	カバーモデル描画
void Bullet::DrawCoverModel()
{
	float coverScale = BulletManager::Instance().GetCoverScale();
	Graphics::Instance().GetShader()->SetBlendState(Shader::BLEND_STATE::ADD);
	coverModel_->GetTransform()->SetScaleFactor(coverScale);
	coverModel_->Render();
}

//	デバッグプリミティブ描画
void Bullet::DrawDebugPrimitive()
{
	DebugRenderer* debugRenderer = Graphics::Instance().GetDebugRenderer();

	//	衝突判定用のデバッグ球を描画
	DirectX::XMFLOAT3 position = this->GetTransform()->GetPosition();
	debugRenderer->DrawSphere(position, radius_, DirectX::XMFLOAT4(0, 0, 0, 1));
}

//	デバッグ描画
void Bullet::DrawDebug()
{
	GetTransform()->DrawDebug();
	ImGui::DragFloat("Radius", &radius_, 1.0f, -FLT_MAX, FLT_MAX);				//	半径

	float scale = GetTransform()->GetScaleFactor();
	ImGui::DragFloat("Scale", &scale, 0.1f, 1.0f, FLT_MAX);						//	スケール
	GetTransform()->SetScaleFactor(scale);

	ImGui::DragFloat("AttackPower", &attackPower_, 0.1f, 1.0f, FLT_MAX);	//	ダメージ量

	//	----- オーディオ -----
	ImGui::DragFloat3("EmitterPosition", &emitter_->position_.x, 0.1f);	//	エミッターの位置
	ImGui::DragFloat("EmitterVolume", &emitter_->volume_, 0.01f);		//	エミッターの音量
}
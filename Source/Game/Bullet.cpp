#include "Bullet.h"

#include "../Nova/Debug/DebugRenderer.h"
#include "../Nova/Graphics/Graphics.h"
#include "../Nova/Collision/Collision.h"
#include "../Nova/Audio/AudioManager.h"
#include "Player.h"

//	コンストラクタ
Bullet::Bullet(const std::string& filename)
{
	//	----- モデル生成 -----
	gltfStaticModelResource_= ResourceManager::Instance().LoadGltfModelStaticResource(filename);

	//	----- 生成時にマネージャーに登録する -----
	BulletManager::Instance().Register(this);

	//	----- 弾丸半径(当たり判定用) -----
	radius_ = 0.5f;

	//	----- スケール -----
	GetTransform()->SetScaleFactor(0.4f);
	
	//	----- 攻撃相手を設定 -----
	opponentType_ = OpponentType::Player;

	//	----- 生存時間 -----
	float lifeTimer_ = 2.5f;

	//	----- オーディオ -----
	emitter_.position_ = GetTransform()->GetPosition();
	//emitter_.velocity_ = velocity_;
	emitter_.velocity_ = { 1,2,1 };
	emitter_.minDistance_ = 7.0f;
	emitter_.maxDistance_ = 12.0f;
	emitter_.volume_ = 1.0f;
	//se_[static_cast<int>(AudioSE3D::Explosion)] = std::unique_ptr<AudioSource3D>(Audio::Instance().LoadAudioSource3D("./Resources/Audio/SE/GameStart_015.wav", emitter_.get()));
	se_[static_cast<int>(Audio3D::Move)] = AudioManager::Instance().LoadAudioSource3D("./Resources/Audio/SE/Bullet/bulletMove.wav", Audio::AudioType::SE3D, "GameScene", &emitter_);
	se_[static_cast<int>(Audio3D::Move)]->SetVolume(0.5f, false);
	se_[static_cast<int>(Audio3D::Move)]->SetAudioName("BulletMove");
	//se_[static_cast<int>(Audio3D::Move)]->SetSceneName("Game");
	se_[static_cast<int>(Audio3D::Move)]->SetDSPSetting(Player::Instance().GetListener());
	AudioManager::Instance().Register(se_[static_cast<int>(Audio3D::Move)]);

	//	----- エフェクト -----
	effectResource_[static_cast<int>(EffectType::Explosion)] = ResourceManager::Instance().LoadEffectResource("./Resources/Effect/Blow11_2.efk");
	effectScale_[static_cast<int>(EffectType::Explosion)] = 0.3f;

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

	//	----- 生存時間更新 -----
	UpdateLifeTimer(elapsedTime);

	//	----- オーディオ関連更新 -----
	UpdateEmitter();
	UpdateAudioSource();

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
	 
	se_[static_cast<int>(Audio3D::Move)]->SetPosition(GetTransform()->GetPosition());	//	AudioSource3Dのemitter_のpositionに渡す
	//emitter_.velocity_ = velocity_;
}

//	オーディオソース更新
void Bullet::UpdateAudioSource()
{
	if (se_[static_cast<int>(Audio3D::Move)])
	{
		se_[static_cast<int>(Audio3D::Move)]->SetDSPSetting(Player::Instance().GetListener());
	}
}

//	カバーモデル更新処理
void Bullet::CoverModelUpdate(const float& elpasedTime)
{
	//	位置更新
	DirectX::XMFLOAT3 bulletPos = this->GetTransform()->GetPosition();
	coverModel_->GetTransform()->SetPosition(bulletPos);
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
	AudioManager::Instance().Remove(se_[static_cast<int>(Audio3D::Move)]);

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
	ImGui::DragFloat3("EmitterPosition", &emitter_.position_.x, 0.1f);	//	エミッターの位置
	ImGui::DragFloat("EmitterVolume", &emitter_.volume_, 0.01f);		//	エミッターの音量
}
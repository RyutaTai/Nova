#include "BulletHorming.h"

#include "../Nova/Others/MathHelper.h"
#include "../Nova/Core/Framework.h"
#include "Player.h"
#include "../Nova/Collision/Collision.h"

//	コンストラクタ
BulletHorming::BulletHorming(const std::string& filename)
	:Bullet(filename)
{
	//	カバーモデル
	DirectX::XMFLOAT4 coverModelColor = { 1.0f,0.0f,0.0f,1.0f };
	coverModel_ = std::make_unique<GltfModelStaticBatching>("./Resources/Model/Cube/source/Cube2.gltf", true, coverModelColor);
	
	coverModel_->GetTransform()->SetScaleFactor(0.4f);

	//	透明処理
#if 1
	//coverModel_->GetTransform()->SetColor({ 1.0f, 1.0f, 1.0f, 0.0f });
#else
	coverModel_->GetTransform()->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
#endif
	//	移動速度設定
	moveSpeed_ = 6.0f;

}

//	デストラクタ
BulletHorming::~BulletHorming()
{

}

//	初期化処理
void BulletHorming::Initialize()
{

}

//	更新処理
void BulletHorming::Update(const float& elapsedTime)
{
	Bullet::Update(elapsedTime);

	//	移動処理
	Move(elapsedTime);

	//	カバーモデル更新処理
	CoverModelUpdate(elapsedTime);

}

//	発射
void BulletHorming::Launch(const DirectX::XMFLOAT3& direction, const DirectX::XMFLOAT3& position)
{
	Bullet::Launch();

	this->direction_ = direction;
	this->GetTransform()->SetPosition(position);

}

//	移動処理
void BulletHorming::Move(const float& elapsedTime)
{
	//	弾丸からプレイヤーまでのベクトルを求め、正規化する
	DirectX::XMFLOAT3 myPos = GetTransform()->GetPosition();
	DirectX::XMFLOAT3 dir = {};
	targetPos_ = Player::Instance().GetTransform()->GetPosition();
	targetPos_.y += Player::Instance().GetHeight() / 1.5f;			//	プレイヤーの拳に当たるようにするため
	dir = targetPos_ - myPos;
	dir = Normalize(dir);											//	正規化

	//	速度計算
	velocity_ = dir * moveSpeed_ * elapsedTime;

	//	ポジション更新
	DirectX::XMFLOAT3 position = GetTransform()->GetPosition();
	position = position + velocity_;
	GetTransform()->SetPosition(position);

}

//	描画処理
void BulletHorming::Render()
{
	//	弾丸モデル描画
	gltfStaticModelResource_->Render();

}

//	カバーモデル描画
void BulletHorming::RnederCoverModel()
{
	float coverScale = BulletManager::Instance().GetCoverScale();
	Graphics::Instance().GetShader()->SetBlendState(Shader::BLEND_STATE::ADD);
	//Graphics::Instance().GetShader()->CreatePsFromCso(Graphics::Instance().GetDevice(), "./Resources/Shader/BulletCoverPS.cso", pixelShader_.ReleaseAndGetAddressOf());
	//coverModel_->SetPixelShader(coverPixelShader_.Get());
	
	//coverModel_->SetPixelShader("./Resources/Shader/GltfModelPS.cso");
	
	coverModel_->SetPixelShaderFromName("./Resources/Shader/BulletCoverPS.cso");
	coverModel_->GetTransform()->SetScaleFactor(coverScale);
	coverModel_->Render();
}

//	デバッグ描画
void BulletHorming::DrawDebug()
{
	if (ImGui::TreeNode(u8"Bullet 弾丸"))
	{
		Bullet::DrawDebug();
		ImGui::DragFloat3("Target", &targetPos_.x, 1.0f, -FLT_MAX, FLT_MAX);			//	ターゲット
		ImGui::DragFloat3("OwnerPos", &ownerPosition_.x, 0.1f, -FLT_MAX, FLT_MAX);	//	所有者の位置
		ImGui::DragFloat("Speed", &moveSpeed_, 0.5f, -FLT_MAX, FLT_MAX);				//	弾の速さ
		ImGui::DragFloat("LifeTimer", &lifeTimer_, 0.5f, -FLT_MAX, FLT_MAX);		//	生存時間
		ImGui::TreePop();
	}
}

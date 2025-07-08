#include "BulletHorming.h"

#include "../../Nova/Others/MathHelper.h"
#include "../../Nova/Core/Framework.h"
#include "../../Nova/Collision/Collision.h"
#include "../Character/Player/Player.h"

BulletHorming::BulletHorming()
	:Bullet()
{
	
}

//	初期化処理
void BulletHorming::Initialize()
{

}

//	更新処理
void BulletHorming::Update(const float& elapsedTime)
{
	//	移動処理
	Move(elapsedTime);

	//	カバーモデル更新処理
	CoverModelUpdate(elapsedTime);

	Bullet::Update(elapsedTime);

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

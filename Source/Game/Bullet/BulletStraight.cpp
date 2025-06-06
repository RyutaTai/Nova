#include "BulletStraight.h"

//	コンストラクタ
BulletStraight::BulletStraight()
	:Bullet()
{
	
}

//	初期化処理
void BulletStraight::Initialize()
{

}

//	更新処理
void BulletStraight::Update(const float& elapsedTime)
{
	//	移動処理
	Move(elapsedTime);

	Bullet::Update(elapsedTime);
	
}

//	発射
void BulletStraight::Launch(const DirectX::XMFLOAT3& direction, const DirectX::XMFLOAT3& position)
{
	Bullet::Launch();

	direction_ = direction;
	GetTransform()->SetPosition(position);
}

//	移動
void BulletStraight::Move(const float& elapsedTime)
{
	//	移動
	float speed = moveSpeed_ * elapsedTime;
	velocity_.x += direction_.x * speed;
	velocity_.y += direction_.y * speed;
	velocity_.z += direction_.z * speed;

}

//	描画処理
void BulletStraight::Render()
{
	gltfStaticModelResource_->Render();
}

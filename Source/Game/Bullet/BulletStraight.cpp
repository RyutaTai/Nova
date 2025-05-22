#include "BulletStraight.h"

//	コンストラクタ
BulletStraight::BulletStraight(const std::string& filename)
	:Bullet(filename)
{
	//	スケール設定
	GetTransform()->SetScaleFactor(30.0f);
}

//	デストラクタ
BulletStraight::~BulletStraight()
{
	
}

//	初期化処理
void BulletStraight::Initialize()
{

}

//	更新処理
void BulletStraight::Update(const float& elapsedTime)
{
	Bullet::Update(elapsedTime);

	//	移動処理
	Move(elapsedTime);
	
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
	DirectX::XMFLOAT3 position = GetTransform()->GetPosition();
	position.x += direction_.x * speed;
	position.y += direction_.y * speed;
	position.z += direction_.z * speed;
	GetTransform()->SetPosition(position);

}

//	描画処理
void BulletStraight::Render()
{
	gltfStaticModelResource_->Render();
}

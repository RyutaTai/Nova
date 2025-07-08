#include "BulletStraight.h"

BulletStraight::BulletStraight()
	:Bullet()
{
	
}

//	‰Šú‰»ˆ—
void BulletStraight::Initialize()
{

}

//	XVˆ—
void BulletStraight::Update(const float& elapsedTime)
{
	//	ˆÚ“®ˆ—
	Move(elapsedTime);

	Bullet::Update(elapsedTime);
	
}

//	”­Ë
void BulletStraight::Launch(const DirectX::XMFLOAT3& direction, const DirectX::XMFLOAT3& position)
{
	Bullet::Launch();

	direction_ = direction;
	GetTransform()->SetPosition(position);
}

//	ˆÚ“®
void BulletStraight::Move(const float& elapsedTime)
{
	//	ˆÚ“®
	float speed = moveSpeed_ * elapsedTime;
	velocity_.x += direction_.x * speed;
	velocity_.y += direction_.y * speed;
	velocity_.z += direction_.z * speed;

}

//	•`‰æˆ—
void BulletStraight::Render()
{
	gltfStaticModelResource_->Render();
}

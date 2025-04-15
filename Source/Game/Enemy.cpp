#include "Enemy.h"

#include "../Nova/Graphics/Graphics.h"
#include "../Nova/Others/MathHelper.h"
#include "Player.h"
#include "EnemyManager.h"

//	コンストラクタ
Enemy::Enemy(const std::string& filename, const std::string& rootNodeName)
	:Character(filename, rootNodeName)
{
	//	エネミーマネージャーに登録
	EnemyManager::Instance().Register(this);

}

//	旋回処理
void Enemy::Turn(const float& elapsedTime)
{
	//	旋回処理しないならreturn
	if (isTurnAction_ == false)return;

	//	ターゲット方向への進行ベクトルを算出(単位ベクトル化はTurn関数内で行っている)
	DirectX::XMFLOAT3 dronePos = this->GetTransform()->GetPosition();
	float vx = targetPosition_.x - dronePos.x;
	float vz = targetPosition_.z - dronePos.z;

	//	旋回処理
#if 1
	Character::Turn(elapsedTime, vx, vz, turnSpeed_);
#endif
}

//	ターゲット位置更新
void Enemy::UpdateTargetPosition()
{
	//	プレイヤーをターゲットに設定
	DirectX::XMFLOAT3 playerPos = Player::Instance().GetTransform()->GetPosition();
	playerPos.y += Player::Instance().GetHeight() / 2.0f;
	SetTargetPosition(playerPos);
}

//	ターゲット位置をランダム設定
void Enemy::SetRandomTargetPosition()
{
	float theta = Mathf::RandomRange(-DirectX::XM_PI, DirectX::XM_PI);
	float range = Mathf::RandomRange(0.0f, territoryRange_);
	targetPosition_.x = territoryOrigin_.x + sinf(theta) * range;
	targetPosition_.y = territoryOrigin_.y;
	targetPosition_.z = territoryOrigin_.z + cosf(theta) * range;
}

//	ターゲットまでの距離
const float Enemy::CalcDistanceToTarget()
{
	float distance = 0.0f;	//	ターゲットまでの距離
	DirectX::XMFLOAT3 position = GetTransform()->GetPosition();		//	自分の位置
	distance = Length(targetPosition_ - position);					//	ターゲットまでの距離算出

	return distance;
}

//	ターゲットに接近する
void Enemy::ApproachingTarget(const float& elapsedTime)
{
	DirectX::XMFLOAT3 position = GetTransform()->GetPosition();

	moveVec_ = Normalize(targetPosition_ - position);

}

//	プレイヤー索敵
bool Enemy::SearchPlayer()
{
	//	プレイヤーとの高低差を考慮して3Dで距離判定をする
	const DirectX::XMFLOAT3& playerPosition = Player::Instance().GetTransform()->GetPosition();
	const DirectX::XMFLOAT3& enemyPosition = this->GetTransform()->GetPosition();
	float vx = playerPosition.x - enemyPosition.x;
	float vy = playerPosition.y - enemyPosition.y;
	float vz = playerPosition.z - enemyPosition.z;
	float dist = sqrtf(vx * vx + vy * vy + vz * vz);

	if (dist < searchRange_)
	{
		float distXZ = sqrtf(vx * vx + vz * vz);
		//	単位ベクトル化
		vx /= distXZ;
		vz /= distXZ;

		//	方向ベクトル化
		float angleY = GetTransform()->GetRotationY();
		float frontX = sinf(angleY);
		float frontZ = cosf(angleY);
		//	2つのベクトルの内積値で前後判定
		float dot = (frontX * vx) + (frontZ * vz);
		if (dot > 0.0f)
		{
			return true;
		}
	}
	return false;
}

//	ダメージ処理
void Enemy::AddDamage(const float& damage)
{
	if (isDead_)return;

	hp_ -= damage;
	if (hp_ <= 0.0f)
	{
		isDead_ = true;
		OnDead();
	}
}

//	破棄
void Enemy::Destroy()
{
	EnemyManager::Instance().Remove(this);
}


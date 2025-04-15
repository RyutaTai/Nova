#include "CollisionManager.h"

#include "CollisionData.h"
#include "../../Game/Player.h"
#include "../../Game/EnemyManager.h"
#include "../../Game/BulletManager.h"
#include "../../Game/Bullet.h"
#include "../../Nova/Audio/AudioManager.h"
#include "../../Nova/Resources/EffectManager.h"
#include "../../Nova/Others/MathHelper.h"
#include "../../imgui/imgui.h"

//  初期化
void CollisionManager::Initialize()
{

}

//  更新処理
void CollisionManager::Update(const float& elapsedTime)
{
    //  Player VS 〇〇
    UpdatePlayerVs(elapsedTime);

    //  Enemy VS 〇〇
    UpdateEnemyVs();

    //  Bullet VS 〇〇
    UpdateBulletVs();

}

//  終了化
void CollisionManager::Finalize()
{

}

//  デバッグ描画
void CollisionManager::DrawDebug()
{
	if (ImGui::TreeNode("CollisionManager"))
	{


		ImGui::TreePop();
	}
}

//	========== Player Vs 〇〇 ==========
#pragma region  ========== Player Vs 〇〇 ==========
void CollisionManager::UpdatePlayerVs(const float& elapsedTime)
{
	//	攻撃判定
	UpdatePlayerAttack();

	//	くらい判定
	UpdatePlayerDamage();

	//	押し出し判定
	UpdatePlayerCollision(elapsedTime);
}

#pragma region	----- 攻撃判定 -----
void CollisionManager::UpdatePlayerAttack()
{
	//  Player VS Enemy
	PlayerAttackVsEnemyDamage();
}

// ----- Player VS Enemy -----
void CollisionManager::PlayerAttackVsEnemyDamage()
{
    //  Enemyが存在しない
    if (EnemyManager::Instance().GetEnemyCount() <= 0) return;
    //  既に攻撃が当たっている
    if (Player::Instance().IsAttackHit()) return;

    Player& player = Player::Instance();

    //  登録されている敵の数だけ処理
    const int maxEnemyCount = EnemyManager::Instance().GetEnemyCount();
    for (int enemyIndex = 0; enemyIndex < maxEnemyCount; ++enemyIndex)
    {
        Enemy* enemy = EnemyManager::Instance().GetEnemy(enemyIndex);
        
        //  当たり判定データの数
        const int maxPlayerData = player.GetAttackDetectionDataCount();
        const int maxEnemyData = enemy->GetDamageDetectionDataCount();

        for (int playerDataIndex = 0; playerDataIndex < maxPlayerData; ++playerDataIndex)
        {
            const AttackDetectionData playerData = player.GetAttackDetectionData(playerDataIndex);
            //  プレイヤーの攻撃判定が無効なら処理しない
            if (playerData.GetIsActive() == false)continue;

            for (int enemyDataIndex = 0; enemyDataIndex < maxEnemyData; ++enemyDataIndex)
            {
                DamageDetectionData enemyData = enemy->GetDamageDetectionData(enemyDataIndex);

                //  このデータは、既にダメージをくらっている
                if (enemyData.IsHit()) continue;

                //  当たったかチェック
                if (IntersectSphereVsSphere(
                    playerData.GetPosition(), playerData.GetRadius(),
                    enemyData.GetPosition(), enemyData.GetRadius()))
                {
                    const Player::StateType playerState = player.GetCurrentState();

                    // ============================================================
                    //  Hitフラグを立てる ( このデータの無敵時間設定 )
                    // ============================================================
                    enemyData.SetIsHit(true);
                    enemyData.SetHitTimer(0.01f);

                     // ============================================================
                    //  プレイヤーのコンボヒット数を増やす
                    // ============================================================
                    player.AddComboCount();

                    // ============================================================
                    // 効果音を鳴らす
                    // ============================================================
                    /*if (playerState != Player::StateType::RushAttack)
                    {
                        AudioManager::Instance().PlaySE(SE::Attack0);
                    }
                    else
                    {
                        AudioManager::Instance().PlaySE(SE::Attack0);
                    }*/

                    // ============================================================
                    // エフェクト再生
                    // ============================================================
                    player.SetPlayEffectFlag(true);
                    player.SetEffectPos(playerData.GetPosition());

                    // ============================================================
                    // 敵が死んでいなかったらダメージ処理をする
                    // ============================================================
                    if (enemy->IsDead() == false)
                    {
                        const float attackPower = player.GetAttackPower();
                        const float damage      = attackPower * enemyData.GetDamage();

                        enemy->SetIsDamaged(true);
                        enemy->AddDamage(damage);

                    }


                    // ============================================================
                    // Playerの攻撃判定を無くす
                    // ============================================================
                    player.SetAttackHit(true);

                    return;
                }
            }
        }
    }
}

#pragma endregion   ----- 攻撃判定 -----

#pragma region  ----- くらい判定 -----
void CollisionManager::UpdatePlayerDamage()
{
    //  Player VS Enemy
    PlayerFlinchVsEnemyAttack(); // 怯み判定
    PlayerDamageVsEnemyAttack(); // くらい判定

    //  Player VS Bullet
    PlayerDamageVsBulletAttack();
}

// ----- Player VS Enemy(プレイヤーの怯み判定) -----
void CollisionManager::PlayerFlinchVsEnemyAttack()
{
    //  Enemyが存在しない
    if (EnemyManager::Instance().GetEnemyCount() <= 0) return;
    //  既にPlayerが怯んでいる
    if (Player::Instance().GetCurrentState() == Player::StateType::Flinch) return;

    Player& player = Player::Instance();

    //  登録されている敵の数だけ処理
    const int maxEnemyCount = EnemyManager::Instance().GetEnemyCount();
    for (int enemyIndex = 0; enemyIndex < maxEnemyCount; ++enemyIndex)
    {
        Enemy* enemy = EnemyManager::Instance().GetEnemy(enemyIndex);

        const int maxPlayerData = player.GetDamageDetectionDataCount();
        const int maxEnemyData = enemy->GetAttackDetectionDataCount();

        for (int playerDataIndex = 0; playerDataIndex < maxPlayerData; ++playerDataIndex)
        {
            const DamageDetectionData playerData = player.GetDamageDetectionData(playerDataIndex);

            for (int enemyDataIndex = 0; enemyDataIndex < maxEnemyData; ++enemyDataIndex)
            {
                const AttackDetectionData enemyData = enemy->GetAttackDetectionData(enemyDataIndex);
                //  このデータは攻撃判定が有効ではない
                if (enemyData.GetIsActive() == false) continue;

                //  当たったか判定
                if (IntersectSphereVsSphere(
                    playerData.GetPosition(), playerData.GetRadius(),
                    enemyData.GetPosition(), enemyData.GetRadius()))
                {
                    //  怯みステートに遷移
                    //player.ChangeState(Player::StateType::Flinch);

                    //  当たったので終了
                    return;
                }
            }
        }
    }
}

//  ----- Player VS Enemy(くらい判定) -----
void CollisionManager::PlayerDamageVsEnemyAttack()
{
    //  Enemyが存在しない
    if (EnemyManager::Instance().GetEnemyCount() <= 0) return;
    //  Playerが無敵状態
    if (Player::Instance().IsInvincible()) return;

    Player& player = Player::Instance();

    //  登録されている敵の数だけ処理
    const int maxEnemyCount = EnemyManager::Instance().GetEnemyCount();
    for (int enemyIndex = 0; enemyIndex < maxEnemyCount; ++enemyIndex)
    {
        Enemy* enemy = EnemyManager::Instance().GetEnemy(enemyIndex);

        const int maxPlayerData = player.GetDamageDetectionDataCount();
        const int maxEnemyData = enemy->GetAttackDetectionDataCount();

        for (int playerDataIndex = 0; playerDataIndex < maxPlayerData; ++playerDataIndex)
        {
            const DamageDetectionData playerData = player.GetDamageDetectionData(playerDataIndex);

            for (int enemyDataIndex = 0; enemyDataIndex < maxEnemyData; ++enemyDataIndex)
            {
                const AttackDetectionData enemyData = enemy->GetAttackDetectionData(enemyDataIndex);

                //  このデータの攻撃判定が有効ではない
                if (enemyData.GetIsActive() == false) continue;

                //  当たったかチェック
                if (IntersectSphereVsSphere(
                    playerData.GetPosition(), playerData.GetRadius(),
                    enemyData.GetPosition(), enemyData.GetRadius()))
                {
                    //  ==================================================
                    //  ダメージを与える
                    //  ==================================================
                    int damage = enemy->GetAttackPower();
                    player.AddDamage(damage);

                    player.SetEnemyPos(enemy->GetTransform()->GetPosition());

                    //  ==================================================
                    //  コントローラー振動 (ダメージ受けたリアクションとして)
                    //  ==================================================
                    /*if (enemy->GetCurrentAttackAction() != Enemy::AttackAction::SuperNova)
                    {
                        Input::Instance().GetGamePad().Vibration(0.2f, 1.0f);
                    }*/

                    //  ==================================================
                    //  ダメージSE再生
                    //  ==================================================
                    //AudioManager::Instance().PlaySE(SE::Damage);

                    //  HPがまだあるためDamageStateに遷移
                    if (player.GetHp() > 0.0f)
                    {
                        player.ChangeState(Player::StateType::Damage);

                    }
                    //  HPが無いためDeathStateに遷移
                    else
                    {
                        player.ChangeState(Player::StateType::Death);  
                    }

                    return;
                }
            }
        }
    }
}

//  ----- Player VS Bullet -----
void CollisionManager::PlayerDamageVsBulletAttack()
{
    //  Bulletが存在していない
    if (BulletManager::Instance().GetBulletCount() <= 0) return;
    //  Playerが無敵状態
    if (Player::Instance().IsInvincible()) return;

    Player& player = Player::Instance();
    std::vector<Bullet*> bullets = BulletManager::Instance().GetBullets();

    const int maxPlayerData = player.GetDamageDetectionDataCount();
    const int maxBulletData = bullets.size();

    for (int playerDataIndex = 0; playerDataIndex < maxPlayerData; ++playerDataIndex)
    {
        const DamageDetectionData playerData = player.GetDamageDetectionData(playerDataIndex);

        for (int bulletIndex = 0; bulletIndex < maxBulletData; ++bulletIndex)
        {
            Bullet* bullet = bullets.at(bulletIndex);

            //  Bulletの攻撃相手がPlayer以外なら当たり判定を行わない
            if (bullet->GetOpponentType() != Bullet::OpponentType::Player) continue;

            //  当たったかチェック
            if (IntersectSphereVsSphere(
                playerData.GetPosition(), playerData.GetRadius(),
                bullet->GetTransform()->GetPosition(), bullet->GetRadius()))
            {
                //bullet->OnHit({});

                //  ダメージを与える
                player.AddDamage(bullet->GetAttackPower());

                player.SetEnemyPos(bullet->GetTransform()->GetPosition());

                //  破棄処理
                bullet->Destroy();

                //  HPがまだあるためDamageStateに遷移
                if (player.GetHp() > 0.0f)
                {
                    player.ChangeState(Player::StateType::Damage);
                }
                //  HPが無いためDeathStateに遷移
                else
                {
                    player.ChangeState(Player::StateType::Death);
                }
                return;
            }
        }
    }
}

#pragma endregion   ----- くらい判定 -----

#pragma region  ----- 押し出し判定 -----
void CollisionManager::UpdatePlayerCollision(const float& elapsedTime)
{
    //  Player VS Enemy
    PlayerVsEnemy(elapsedTime);
}

//  ----- Player VS Enemy -----
void CollisionManager::PlayerVsEnemy(const float& elapsedTime)
{
    //  Enemyが存在しない
    if (EnemyManager::Instance().GetEnemyCount() <= 0) return;
    //  押し出し判定が無効
    if (Player::Instance().IsActiveCollisionDetection() == false) return;
    
    Player& player = Player::Instance();

    //  登録されている敵の数だけ処理
    const int maxEnemyCount = EnemyManager::Instance().GetEnemyCount();
    for (int enemyIndex = 0; enemyIndex < maxEnemyCount; ++enemyIndex)
    {
        Enemy* enemy = EnemyManager::Instance().GetEnemy(enemyIndex);

        const int maxPlayerData = player.GetCollisionDetectionDataCount();
        const int maxEnemyData = enemy->GetCollisionDetectionDataCount();

        for (int playerDataIndex = 0; playerDataIndex < maxPlayerData; ++playerDataIndex)
        {
            const CollisionDetectionData playerData = player.GetCollisionDetectionData(playerDataIndex);

            for (int enemyDataIndex = 0; enemyDataIndex < maxEnemyData; ++enemyDataIndex)
            {
                const CollisionDetectionData enemyData = enemy->GetCollisionDetectionData(enemyDataIndex);

                //  このデータの判定が無効
                if (enemyData.GetIsActive() == false) continue;

                //  押し出し後の位置
                DirectX::XMFLOAT3 resultPosition = {};

                //  Yの値が0.0fのデータとの判定
                if (enemyData.GetFixedY())
                {
                    //  当たったかチェック
                    if (IntersectSphereVsSphere(
                        enemyData.GetPosition(), enemyData.GetRadius(),
                        player.GetTransform()->GetPosition(), playerData.GetRadius(),
                        resultPosition))
                    {
                        resultPosition.y = 0.0f;

                        player.GetTransform()->SetPosition(resultPosition);
                    }
                }
                //  その他のデータとの判定
                else
                {
                    //  当たったかチェック
                    if (IntersectSphereVsSphereNotConsiderY(
                        enemyData.GetPosition(), enemyData.GetRadius(),
                        playerData.GetPosition(), playerData.GetRadius(),
                        resultPosition))
                    {
                        resultPosition = player.GetTransform()->GetPosition() - resultPosition;
                        player.GetTransform()->SetPosition(resultPosition);
                    }
                }
            }
        }
    }
}

#pragma endregion   ----- 押し出し判定 -----
#pragma endregion   ========== Player Vs 〇〇 ==========

//  ========== Enemy Vs 〇〇 ==========
#pragma region  ========== Enemy Vs 〇〇 ==========
void CollisionManager::UpdateEnemyVs()
{
    //  くらい判定
    UpdateEnemyDamage();
}

#pragma region	----- くらい判定 -----
//  ----- くらい判定 -----
void CollisionManager::UpdateEnemyDamage()
{
    //  Enemy VS Bullet
    EnemyDamageVsBulletAttack();
}

void CollisionManager::EnemyDamageVsBulletAttack()
{
    //  Enemyが存在しない
    if (EnemyManager::Instance().GetEnemyCount() <= 0) return;
    //  Bulletが存在しない
    if (BulletManager::Instance().GetBulletCount() <= 0) return;

    //  登録されている敵の数だけ処理
    const int maxEnemyCount = EnemyManager::Instance().GetEnemyCount();
    for (int enemyIndex = 0; enemyIndex < maxEnemyCount; ++enemyIndex)
    {
        Enemy* enemy = EnemyManager::Instance().GetEnemy(enemyIndex);
        std::vector<Bullet*> bullets = BulletManager::Instance().GetBullets();

        const int maxEnemyData = enemy->GetDamageDetectionDataCount();
        const int maxBulletData = bullets.size();

        for (int enemyDataIndex = 0; enemyDataIndex < maxEnemyData; ++enemyDataIndex)
        {
            DamageDetectionData enemyData = enemy->GetDamageDetectionData(enemyDataIndex);

            //  このデータは、既にダメージをくらっている
            if (enemyData.IsHit()) continue;

            for (int bulletIndex = 0; bulletIndex < maxBulletData; ++bulletIndex)
            {
                Bullet* bullet = bullets.at(bulletIndex);

                //  Bulletの攻撃相手がEnemy以外なら当たり判定を行わない
                if (bullet->GetOpponentType() != Bullet::OpponentType::Enemy) continue;

                //  当たったかチェック
                if (IntersectSphereVsSphere(
                    bullet->GetTransform()->GetPosition(), bullet->GetRadius(),
                    enemyData.GetPosition(), enemyData.GetRadius()))
                {
                    //  当たった位置を求める
                    const DirectX::XMFLOAT3 hitPosition = bullet->GetTransform()->GetPosition() + Normalize(enemyData.GetPosition() - bullet->GetTransform()->GetPosition()) * bullet->GetRadius();

                    // ============================================================
                    //  Hitフラグ & 関数呼び出し
                    // ============================================================
                    enemyData.SetIsHit(true);
                    enemyData.SetHitTimer(0.01f);
                    //bullet->OnHit(hitPosition);

                    // ============================================================
                    // 敵が死んでいなかったらダメージ処理をする
                    // ============================================================
                    if (enemy->IsDead() == false)
                    {
                        const float attackPower = bullet->GetAttackPower();
                        const float damage = attackPower * enemyData.GetDamage();

                        enemy->AddDamage(damage);


                    }

                    //  破棄処理
                    bullet->Destroy();

                    return;
                }
            }
        }
    }
}

#pragma endregion	----- くらい判定 -----

#pragma endregion  ========== Enemy Vs 〇〇 ==========

//  ==================== Bullet VS 〇〇 ====================
#pragma region  ==================== Bullet VS 〇〇 ====================
void CollisionManager::UpdateBulletVs()
{
}

#pragma endregion   ==================== Bullet VS 〇〇 ====================

//  ========== Intersect ==========
#pragma region  ========== Intersect ==========
//  球と球の交差判定(押し出しあり)
bool CollisionManager::IntersectSphereVsSphere(const DirectX::XMFLOAT3& positionA, const float& radiusA, const DirectX::XMFLOAT3& positionB, const float& radiusB, DirectX::XMFLOAT3& outPositionB)
{
#if 0
    //	A->Bの単位ベクトルを算出
    DirectX::XMVECTOR PositionA = DirectX::XMLoadFloat3(&positionA);
    DirectX::XMVECTOR PositionB = DirectX::XMLoadFloat3(&positionB);
    DirectX::XMVECTOR Vec = DirectX::XMVectorSubtract(PositionB, PositionA);
    float lengthSq = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(Vec));//ルートを取っていない状態（2乗）

    //	距離判定
    float range = radiusA + radiusB;
    //	当たっていない
    if (lengthSq > range * range)return false;

    //	AがBを押し出す
    DirectX::XMVECTOR OutPositionB;
    Vec = DirectX::XMVector3Normalize(Vec);
    Vec = DirectX::XMVectorScale(Vec, range);
    OutPositionB = DirectX::XMVectorAdd(PositionA, Vec);
    DirectX::XMStoreFloat3(&outPositionB, OutPositionB);
#else
    float vx = positionB.x - positionA.x;
    float vz = positionB.z - positionA.z;
    const float range = radiusA + radiusB;
    const float distXZ = sqrtf(vx * vx + vz * vz);

    // 当たっていない
    if (distXZ > range) return false;

    // 正規化
    vx /= distXZ;
    vz /= distXZ;

    outPositionB.x = positionA.x + (vx * range);
    outPositionB.y = positionB.y;
    outPositionB.z = positionA.z + (vz * range);

#endif

    return true;

}

//	球と球の交差判定(押し出しなし)
bool CollisionManager::IntersectSphereVsSphere(const DirectX::XMFLOAT3& positionA, const float& radiusA, const DirectX::XMFLOAT3& positionB, const float& radiusB)
{
    //	A->Bの単位ベクトルを算出
    DirectX::XMVECTOR PositionA = DirectX::XMLoadFloat3(&positionA);
    DirectX::XMVECTOR PositionB = DirectX::XMLoadFloat3(&positionB);
    DirectX::XMVECTOR Vec = DirectX::XMVectorSubtract(PositionB, PositionA);
    float lengthSq = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(Vec));//ルートを取っていない状態（2乗）

    //	距離判定
    float range = radiusA + radiusB;
    //	当たっていない
    if (lengthSq > range * range)return false;

    return true;
}

//  球と球の当たり判定(Y軸方向の押し出しなし)
const bool CollisionManager::IntersectSphereVsSphereNotConsiderY(const DirectX::XMFLOAT3& positionA, const float& radiusA, const DirectX::XMFLOAT3& positionB, const float& radiusB, DirectX::XMFLOAT3& outPositionB)
{
    DirectX::XMVECTOR PositionA = DirectX::XMLoadFloat3(&positionA);
    DirectX::XMVECTOR PositionB = DirectX::XMLoadFloat3(&positionB);
    DirectX::XMVECTOR Vec = DirectX::XMVectorSubtract(PositionA, PositionB);
    DirectX::XMVECTOR LengthSq = DirectX::XMVector3LengthSq(Vec);
    float lengthSq;
    DirectX::XMStoreFloat(&lengthSq, LengthSq);

    const float range = radiusA + radiusB;
    // 当たっていない
    if (lengthSq > range * range) return false;

    DirectX::XMVECTOR HorizonVec = DirectX::XMVectorSetY(Vec, 0.0f);

    const float lengthHorizon = DirectX::XMVectorGetX(DirectX::XMVector3Length(HorizonVec));
    const float subY = DirectX::XMVectorGetY(Vec);
    const float newHorizonLength = sqrtf(range * range - subY * subY);
    const DirectX::XMVECTOR normalVec = DirectX::XMVector3Normalize(HorizonVec);
    const float penetration = newHorizonLength - lengthHorizon;

    DirectX::XMStoreFloat3(&outPositionB, DirectX::XMVectorScale(normalVec, penetration));

    return true;
}

//	円柱と円柱の交差判定
bool CollisionManager::IntersectCylinderVsCyliner(const DirectX::XMFLOAT3& positionA, const float& radiusA, const float& heightA, const DirectX::XMFLOAT3& positionB, const float& radiusB, const float& heightB, DirectX::XMFLOAT3& outPosition, const bool& outIsA)
{
    //	Aの足元がBの頭より上なら当たっていない
    if (positionA.y > positionB.y + heightB)return false;

    //	Aの頭がBの足元より下なら当たっていない
    if (positionA.y + heightA < positionB.y)return false;

    //	XZ平面での範囲チェック
    DirectX::XMVECTOR PositionA = DirectX::XMLoadFloat3(&positionA);
    DirectX::XMVECTOR PositionB = DirectX::XMLoadFloat3(&positionB);
    DirectX::XMVECTOR Vec = DirectX::XMVectorSubtract(PositionB, PositionA);//PositionAとBが逆だった
    DirectX::XMVECTOR Length = DirectX::XMVector3LengthSq(Vec);
    float length;
    DirectX::XMStoreFloat(&length, Length);
    float range = radiusA + radiusB;
    if (length > range * range)return false;

    if (outIsA)
    {
        //	BがAを押し出す
        DirectX::XMVECTOR OutPositionA;
        OutPositionA = DirectX::XMVectorSubtract(PositionB, DirectX::XMVectorScale(DirectX::XMVector3Normalize(Vec), range));
        DirectX::XMStoreFloat3(&outPosition, OutPositionA);
    }
    else
    {
        //	AがBを押し出す
        DirectX::XMVECTOR OutPositionB;
        OutPositionB = DirectX::XMVectorAdd(PositionA, DirectX::XMVectorScale(DirectX::XMVector3Normalize(Vec), range));
        DirectX::XMStoreFloat3(&outPosition, OutPositionB);
    }

    return true;

}

//	球と円柱の交差判定
bool CollisionManager::IntersectSphereVsCylinder(const DirectX::XMFLOAT3& spherePosition, const float& sphereRadius, const DirectX::XMFLOAT3& cylinderPosition, const float& cylinderRadius, const float& cylinderHeight, DirectX::XMFLOAT3& outCylinderPosition)
{
    //	球の位置が円柱の位置より上なら当たっていない
    if (spherePosition.y - sphereRadius > cylinderPosition.y + cylinderHeight)
        return false;

    //	球の位置が円柱の位置より下なら当たっていない
    if (spherePosition.y + sphereRadius < cylinderPosition.y)
        return false;

    //	A->Bの単位ベクトルを算出
    DirectX::XMVECTOR PositionA = DirectX::XMLoadFloat3(&spherePosition);
    DirectX::XMVECTOR PositionB = DirectX::XMLoadFloat3(&cylinderPosition);
    DirectX::XMVECTOR Vec = DirectX::XMVectorSubtract(PositionB, PositionA);
    DirectX::XMVECTOR LengthSq = DirectX::XMVector3LengthSq(Vec);//	ルートを取っていない状態（2乗）
    float lengthSq;
    DirectX::XMStoreFloat(&lengthSq, LengthSq);
    //	距離判定
    float range = sphereRadius + cylinderRadius;
    if (lengthSq > range * range)return false;

    return true;
}

#pragma endregion  ========== Intersect ==========
#include "EnemyManager.h"

#include "../Nova/Collision/Collision.h"
#include "../Nova/Others/MathHelper.h"
#include "Player.h"
#include "Drone.h"

//	更新処理
void EnemyManager::Update(const float& elapsedTime)
{
	for (Enemy* enemy : enemies_)
	{
		enemy->Update(elapsedTime);
	}

	//	破棄処理
	for (Enemy* enemy : removes_)
	{
		std::vector<Enemy*>::iterator it =
			std::find(enemies_.begin(), enemies_.end(), enemy);

		if (it != enemies_.end())
		{
			enemies_.erase(it);
		}

		//	弾丸処理
		//delete enemy;
	}

	//	破棄リストをクリア
	removes_.clear();

	//	敵同士の衝突処理
	CollisionEnemyVsEnemies();

}

//	円柱と円柱
void EnemyManager::CollisionEnemyVsEnemies()
{
	EnemyManager& enemyManager = EnemyManager::Instance();
	int EnemyCount = GetEnemyCount();

	//	全ての敵との総当たりで衝突処理
	for (int i = 0; i < EnemyCount; i++)
	{
		Enemy* enemy = enemyManager.GetEnemy(i);
		for (int n = 0; n < EnemyCount; n++)
		{
			Enemy* enemy2 = enemyManager.GetEnemy(n);
			DirectX::XMFLOAT3 outPosition;
			DirectX::XMFLOAT3 positionOffset = {};
			if (Collision::IntersectCylinderVsCyliner(
				enemy->GetTransform()->GetPosition() + positionOffset,
				enemy->GetRadius(),
				enemy->GetHeight(),
				enemy2->GetTransform()->GetPosition() + positionOffset, 
				enemy2->GetRadius(),
				enemy2->GetHeight(),
				outPosition
			))
			{
				enemy2->GetTransform()->SetPosition(outPosition);
			}
		}
	}
}

//	エネミー登録
void EnemyManager::Register(Enemy* enemy)
{
	enemies_.emplace_back(enemy);
}

//	エネミー全削除
void EnemyManager::Clear()
{
	/*for (Enemy* enemy : enemies_)
	{
		delete enemy;
	}*/
	enemies_.clear();
}

//	エネミー削除
void EnemyManager::Remove(Enemy* enemy)
{
	//	破棄リストに追加
	removes_.insert(enemy);
}

//	ドローン生成(GameStateで呼んでいる)
#if 0
void EnemyManager::DroneSpawn(const int& spawn)
{
	DirectX::XMFLOAT3 playerPos = Player::Instance().GetTransform()->GetPosition();
	DirectX::XMFLOAT3 playerForward = Player::Instance().GetTransform()->CalcForward();
	constexpr float spaceWithP = 2.0f;		//	プレイヤーとの間隔
	DirectX::XMFLOAT3 emitter = playerPos + playerForward * spaceWithP;
	DirectX::XMFLOAT3 spawnOffset = { 5.0f,0.0f,5.0f };
	
	for (int spawnCount = 0; spawnCount < spawn; spawnCount++)
	{
		Drone* drone = new Drone();	//	生成時に登録される
		//	生成位置設定
		DirectX::XMFLOAT3 pos =
		{
			emitter.x + spawnOffset.x * spawnCount,
			7.0f,
			emitter.z + spawnOffset.z * spawnCount
		};
		drone->GetTransform()->SetPosition(pos);
		drone->Initialize();	//	ドローン初期化
	}
}
#else
void EnemyManager::DroneSpawn(const int& spawn)
{
	for (int spawnCount = 0; spawnCount < spawn; spawnCount++)
	{
		Drone* drone = new Drone();	//	生成時に登録される
		//	生成位置設定
		DirectX::XMFLOAT3 pos =
		{
			51.0f, 2.5f, -7.0f
		};
		drone->GetTransform()->SetPosition(pos);
		drone->Initialize();	//	ドローン初期化
	}
}
#endif

//	描画処理
void EnemyManager::Render()
{
	for (Enemy* enemy : enemies_)
	{
		enemy->Render();
	}

}

//	デバッグプリミティブ描画
void EnemyManager::DrawDebugPrimitive()
{
	for (Enemy* enemy : enemies_)
	{
		enemy->DrawDebugPrimitive();
	}
}

//	デバッグ描画
void EnemyManager::DrawDebug()
{
	int enemyCount = static_cast<int>(enemies_.size());

	if (ImGui::TreeNode("EnemyManager"))
	{
		//	EnemyManagerのデバッグ描画
		ImGui::DragInt("EnemyCount", &enemyCount);	//	エネミーの総数

		//	Enemyのデバッグ描画
		for (Enemy* enemy : enemies_)
		{
			enemy->DrawDebug();
		}
		ImGui::TreePop();
	}
}
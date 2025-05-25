#include "EnemyManager.h"

#include "../../../Nova/Collision/Collision.h"
#include "../../../Nova/Others/MathHelper.h"
#include "../Player/Player.h"
#include "../Enemy/Drone/Drone.h"

//	更新処理
void EnemyManager::Update(const float& elapsedTime)
{
	for (const auto& enemy : enemies_)
	{
		enemy->Update(elapsedTime);
	}

	// 削除対象のEnemyをリストから削除
	// unique_ptrのリストから要素を削除する際は、remove_ifとeraseを組み合わせる
	enemies_.erase(std::remove_if(enemies_.begin(), enemies_.end(),
		[this](const std::unique_ptr<Enemy>& p) {
			// removeEnemies_ に存在する生ポインタと一致する unique_ptr を探す
			// p.get() で unique_ptr が保持する生ポインタを取得
			return std::find(removes_.begin(), removes_.end(), p.get()) != removes_.end();
		}),
		enemies_.end());
	removes_.clear(); // 削除リストをクリア

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
void EnemyManager::Register(std::unique_ptr<Enemy> enemy)
{
	//	所有権を移動してvectorに格納する
	if (enemy)enemies_.emplace_back(std::move(enemy));
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
	// 即座に削除するのではなく、次のUpdateでまとめて削除するためにリストに追加
   // これにより、イテレータの無効化問題を回避できる
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
		std::unique_ptr<Drone> drone = std::make_unique<Drone>();	//	生成時に登録される
		//	生成位置設定
		DirectX::XMFLOAT3 pos =
		{
			51.0f, 2.5f, -7.0f
		};
		drone->GetTransform()->SetPosition(pos);
		drone->Initialize();	//	ドローン初期化

		Register(std::move(drone));
	}
}
#endif

//	描画処理
void EnemyManager::Render()
{
	for (const auto& enemy : enemies_)
	{
		enemy->Render();
	}
}

//	シャドウマップ
void EnemyManager::CastShadows()
{
	for (const auto& enemy : enemies_)
	{
		enemy->CastShadows();
	}
}

//	デバッグプリミティブ描画
void EnemyManager::DrawDebugPrimitive()
{
	for (const auto& enemy : enemies_)
	{
		enemy->DrawDebugPrimitive();
	}
}

//	エネミーを要素番号を指定して取得
Enemy* EnemyManager::GetEnemy(const int& index)
{
	if (index < 0 || index >= enemies_.size())
	{

		_ASSERT_EXPR(false, L"Enemy index is out of bounds."); // あなたのプロジェクトのAssertを使う
		return nullptr; // あるいは例外を投げる
	}
	return enemies_.at(index).get(); // unique_ptrから生ポインタを取得
}

//	エネミーを取得
std::vector<Enemy*> EnemyManager::GetEnemies()
{
	std::vector<Enemy*> rawPointers;
	rawPointers.reserve(enemies_.size()); // メモリ再割り当てを避けるため
	for (const auto& uptr : enemies_)
	{
		rawPointers.push_back(uptr.get()); // unique_ptrから生ポインタを取得
	}
	return rawPointers;
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
		for (const auto& enemy : enemies_)
		{
			enemy->DrawDebug();
		}
		ImGui::TreePop();
	}
}
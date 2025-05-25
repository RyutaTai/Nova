#pragma once

#include <vector>
#include <set>

#include "Enemy.h"

//	エネミー管理クラス
class EnemyManager
{
private:
	EnemyManager() {}
	~EnemyManager() {}

public:
	static EnemyManager& Instance()
	{
		static EnemyManager instance;
		return instance;
	}

	void Update(const float& elapsedTime);
	void Render();

	void Register(std::unique_ptr<Enemy> enemy);					//	エネミー登録
	void Clear();									//	エネミー全削除
	void Remove(Enemy* enemy);						//	エネミー削除
	void DroneSpawn(const int& spawn/*生成数*/);	//	ドローン生成

	//	シャドウマップ
	void CastShadows();

	void DrawDebug();
	void DrawDebugPrimitive();

	Enemy* GetEnemy(const int& index);		//	エネミー取得
	std::vector<Enemy*> GetEnemies();		//	エネミー取得
	int					GetEnemyCount() const { return static_cast<int>(enemies_.size()); }	//	エネミー数取得

private:
	void CollisionEnemyVsEnemies();		//	エネミー同士の衝突判定

private:
	std::vector	<std::unique_ptr<Enemy>> enemies_ = {};		//	エネミーの配列
	std::set	<Enemy*> removes_ = {};		//	破棄するエネミーの配列

};

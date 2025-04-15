#pragma once

#include <memory>

#include "Bullet.h"
#include "../Nova/Resources/Effect.h"

class BulletHorming :public Bullet
{
public:
	BulletHorming(const std::string& filename);
	~BulletHorming()override;

	void Initialize()override;
	void Update(const float& elapsedTime)override;
	
	//	----- 描画処理 -----
	void Render()override;
	void RnederCoverModel()override;

	//	----- デバッグ描画 -----
	void DrawDebug();

	//	----- 発射 -----
	void Launch(const DirectX::XMFLOAT3& direction, const DirectX::XMFLOAT3& position)override;
	
	//	----- 移動 -----
	void Move(const float& elpasedTime);

};


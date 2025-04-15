#pragma once

#include "Bullet.h"

class BulletManager;

//	’¼i’eŠÛ
class BulletStraight :public Bullet
{
public:
	BulletStraight(const std::string& filename);
	~BulletStraight()override;

	void Initialize()override;
	void Update(const float& elapsedTime)override;

	//	----- •`‰æˆ— -----
	void Render()override;
	void RnederCoverModel()override {}

	//	----- ”­Ë -----
	void Launch(const DirectX::XMFLOAT3& direction, const DirectX::XMFLOAT3& position)override;

	//	----- ˆÚ“® -----
	void Move(const float& elapsedTime);

};
#pragma once

#include "Bullet.h"

class BulletManager;

//	’¼i‚·‚é’eŠÛ‚ÌƒNƒ‰ƒX
class BulletStraight :public Bullet
{
public:
	BulletStraight();
	~BulletStraight()override = default;

	void Initialize()override;
	void Update(const float& elapsedTime)override;

	//	----- •`‰æˆ— -----
	void Render()override;

	//	----- ”­Ë -----
	void Launch(const DirectX::XMFLOAT3& direction, const DirectX::XMFLOAT3& position)override;

	//	----- ˆÚ“® -----
	void Move(const float& elapsedTime);

};
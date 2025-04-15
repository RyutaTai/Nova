#pragma once

#include <DirectXMath.h>

class CollisionManager
{
public:
	//	”»’è‚Ìí—Ş
	enum class CollisionType
	{
		Attack,
		Max
	};

private:
	CollisionManager(){}
	~CollisionManager() {}

public:
	static CollisionManager& Instance()
	{
		static CollisionManager instance;
		return instance;
	}

	void Initialize();
	void Update(const float& elapsedTime);
	void Finalize();
	void DrawDebug();

private:
#pragma region	========== Player Vs ZZ ==========
	void UpdatePlayerVs(const float& elapsedTime);
	//	UŒ‚”»’è
	void UpdatePlayerAttack();				//	PlayerAttack
	void PlayerAttackVsEnemyDamage();		//	Player Vs Enemy
	//	‚­‚ç‚¢”»’è
	void UpdatePlayerDamage();              //	PlayerDamage
	void PlayerFlinchVsEnemyAttack();       //	Player Vs Enemy(‹¯‚İ”»’è)
	void PlayerDamageVsEnemyAttack();       //	Player Vs Enemy(‚­‚ç‚¢”»’è)
	void PlayerDamageVsBulletAttack();		//	Player Vs Bullet
	
	// ”»’è
	void UpdatePlayerCollision(const float& elapsedTime);           //	PlayerCollision
	void PlayerVsEnemy(const float& elapsedTime);                   //	Player Vs Enemy

#pragma endregion	========== Player Vs ZZ ==========

#pragma region	========== Enemy Vs ZZ ==========
	void UpdateEnemyVs();
	// ----- ‚­‚ç‚¢”»’è -----
	void UpdateEnemyDamage();               // EnemyDamage
	void EnemyDamageVsBulletAttack();		// Enemy VS Bullet

#pragma endregion	========== Enemy Vs ZZ ==========

#pragma region	========== Bullet Vs ZZ ==========
	void UpdateBulletVs();

#pragma endregion	========== Bullet Vs ZZ ==========

private:	//	”»’è—pŠÖ”
#pragma region	========== Intersect ==========
	//	‹…‚Æ‹…‚ÌŒğ·”»’è(‰Ÿ‚µo‚µ‚ ‚è)
	bool IntersectSphereVsSphere(const DirectX::XMFLOAT3& positionA, const float& radiusA,
		const DirectX::XMFLOAT3& positionB, const float& radiusB,
		DirectX::XMFLOAT3& outPositionB);
	//	‹…‚Æ‹…‚ÌŒğ·”»’è(‰Ÿ‚µo‚µ‚È‚µ)
	bool IntersectSphereVsSphere(const DirectX::XMFLOAT3& positionA, const float& radiusA,
		const DirectX::XMFLOAT3& positionB, const float& radiusB);
	//	‹…‚Æ‹…‚Ì“–‚½‚è”»’è(Y²•ûŒü‚Ì‰Ÿ‚µo‚µ‚È‚µ)
	const bool IntersectSphereVsSphereNotConsiderY(
		const DirectX::XMFLOAT3& positionA, const float& radiusA,
		const DirectX::XMFLOAT3& positionB, const float& radiusB,
		DirectX::XMFLOAT3& outPositionB);
	//	‰~’Œ‚Æ‰~’Œ‚ÌŒğ·”»’è
	bool IntersectCylinderVsCyliner(const DirectX::XMFLOAT3& positionA, const float& radiusA, const float& heightA,
		const DirectX::XMFLOAT3& positionB, const float& radiusB, const float& heightB,
		DirectX::XMFLOAT3& outPosition, const bool& outIsA = false/*A‚ğ‰Ÿ‚·‚©B‚ğ‰Ÿ‚·‚©*/);

	//	‹…‚Æ‰~’Œ‚ÌŒğ·”»’è
	bool IntersectSphereVsCylinder(const DirectX::XMFLOAT3& spherePosition, const float& sphereRadius,
		const DirectX::XMFLOAT3& cylinderPosition, const float& cylinderRadius, const float& cylinderHeight,
		DirectX::XMFLOAT3& outCylinderPosition);

#pragma endregion	========== Intersect ==========

};


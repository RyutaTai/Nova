#pragma once

#include "Enemy.h"
#include "../Nova/AI/BehaviorTree.h"
#include "../Nova/AI/NodeBase.h"

class BehaviorTree;
class BehaviorData;
class NodeBase;

class Dragonkin : public Enemy
{
public:
	Dragonkin();
	~Dragonkin()override {}

public:
	//	アニメーション情報(Boss)
	enum class AnimationType
	{
		AttackPunch = 0, 	//	パンチ
		AttackKick,			//	キック
		AttackWing,			//	翼攻撃
		Attack04,
		Init01,
		Init02,
		Init03,
		Run,
		Skil01,
		Skil02,
		Skil03,
		Skil04,
		Skil05,
		Skil06,
		Skil07,
		Skil08,
		Idle01,				//	待機
		Idle02,				//	待機2
		Walk,
		DmageDie01,
		DmageDie02,
		DmageDie03,
		DmageDieDown,
		DamageDown01,
		DamageDown02,
		DamageHit01,
		DamageHit02,
		STU01,
		STU02,
		STU03,
		BD01,
		BD02,
		BD03,
		BD04,
		BD05,
		Cine02,
		Cine03,
		Max,
	};

public:
	void Initialize()override;
	void Update(const float& elapsedTime)override;
	void Render()override;

	bool RayVsVertical(const float& elapsedTime)override;
	bool RayVsHorizontal(const float& elapsedTime)override;
	void Move(const float& elapsedTime)override {}

	void DrawDebug()override;
	void DrawDebugPrimitive()override;

	//	----- 破棄処理 -----
	void JudgeDestroy();
	void Destroy()override;

	//	----- アニメーション -----
	void PlayAnimation(const AnimationType& animType, const bool& loop = false, const float& blendTime = 1.0f, const float& animSpeed = 1.0f, const float& startFrame = 0.0f, const float& endFrame = 0.0f);

	//	----- Collision -----
	void UpdateCollisions(const float& elapsedTime)override;
	void RegisterCollisionData()override;

	//	----- ビヘイビアツリー -----
	const std::string GetActiveNodeName()const { return activeNode_->GetName(); }

private:
	//	----- ビヘイビアツリー -----
	void UpdateBehaviorTree(const float& elapsedTime);	//	ビヘイビアツリー更新

private:
	static const int MaxHp_ = 70;

	BehaviorTree*	behaviorTree_	= nullptr;
	BehaviorData*	behaviorData_	= nullptr;
	NodeBase*		activeNode_		= nullptr;

private:	//	デバッグ用
	//	----- DebugPrimitive -----
	bool isCollisionSphere_ = true;
	bool isAttackSphere_ = true;
	bool isDamageSphere_ = false;

	bool updateFlag_ = true;				//	更新フラグ
	bool behaviorTreeUpdateFlag_ = true;	//	ビヘイビアツリー更新フラグ

};


#pragma once

#include "Dragonkin.h"
#include "../Nova/AI/ActionBase.h"
#include "../Nova/Others/JudgeTime.h"

//	待機行動
namespace DragonkinAction
{
	class IdleAction : public ActionBase
	{
	public:
		IdleAction(Dragonkin* owner) :ActionBase(owner) {}
		ActionBase::State Run(const float& elapsedTime)override;
		void DrawDebug()override;

	private:
		float idleTimer_ = 1.0f;	//	待機する時間

	};
}

//	索敵行動
namespace DragonkinAction
{
	class SearchAction : public ActionBase
	{
	public:
		SearchAction(Dragonkin* owner) :ActionBase(owner) {}
		ActionBase::State Run(const float& elapsedTime)override;
		void DrawDebug()override;

	};
}

#pragma region ===== 攻撃 =====
#pragma region ----- 通常攻撃 -----
//	通常殴打
namespace DragonkinAction
{
	class AttackPunchAction :public ActionBase
	{
	public:
		AttackPunchAction(Dragonkin* owner):ActionBase(owner){}
		ActionBase::State Run(const float& elapsedTime)override;
		void DrawDebug()override;

	private:
		JudgeTime	animJudgeTime_ = {};	//	判定時間

	};
}

//	通常キック
namespace DragonkinAction
{
	class AttackKickAction :public ActionBase
	{
	public:
		AttackKickAction(Dragonkin* owner) :ActionBase(owner) {}
		ActionBase::State Run(const float& elapsedTime)override;
		void DrawDebug()override;

	private:
		JudgeTime	animJudgeTime_ = {};	//	判定時間

	};
}

//	通常翼攻撃
namespace DragonkinAction
{
	class AttackWingAction :public ActionBase
	{
	public:
		AttackWingAction(Dragonkin* owner) :ActionBase(owner) {}
		ActionBase::State Run(const float& elapsedTime)override;
		void DrawDebug()override;

	private:
		JudgeTime	animJudgeTime_ = {};	//	判定時間

	};
}
#pragma endregion ----- 通常攻撃 -----

#pragma region ----- スキル攻撃 -----
//	スキル攻撃行動
namespace DragonkinAction
{
	class SkillAction : public ActionBase
	{
	public:
		SkillAction(Dragonkin* owner) :ActionBase(owner) {}
		ActionBase::State Run(const float& elapsedTime)override;
		void DrawDebug()override;

	private:
		JudgeTime	animJudgeTime_ = {};	//	判定時間

	};
}
#pragma endregion ----- スキル攻撃 -----
#pragma endregion ===== 攻撃 =====

//	追跡行動
namespace DragonkinAction
{
	class PursuitAction : public ActionBase
	{
	public:
		PursuitAction(Dragonkin* enemy) :ActionBase(enemy) {}
		ActionBase::State Run(const float& elapsedTime)override;
		void DrawDebug()override;
	};
}

//	逃走行動
namespace DragonkinAction
{
	class LeaveAction : public ActionBase
	{
	public:
		LeaveAction(Dragonkin* owner) :ActionBase(owner) {}
		ActionBase::State Run(const float& elapsedTime)override;
		void DrawDebug()override;
	};
}

//	回復行動
namespace DragonkinAction
{
	class RecoverAction : public ActionBase
	{
	public:
		RecoverAction(Dragonkin* owner) :ActionBase(owner) {}
		ActionBase::State Run(const float& elapsedTime)override;
		void DrawDebug()override;
	};
}

//	ダメージ行動
namespace DragonkinAction
{
	class DamageAction :public ActionBase
	{
	public:
		DamageAction(Dragonkin* owner) :ActionBase(owner) {}
		ActionBase::State Run(const float& elapsedTime)override;
		void DrawDebug()override;

	private:
		//	----- アニメーション -----
		float startFrame_ = 0.0f;
		float endFrame_ = 0.333f;

	};
}

//	死亡行動
namespace DragonkinAction
{
	class DeathAction :public ActionBase
	{
	public:
		DeathAction(Dragonkin* owner) :ActionBase(owner) {}
		ActionBase::State Run(const float& elapsedTime)override;
		void DrawDebug()override;

	};
}
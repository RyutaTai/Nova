#pragma once

#include "Dragonkin.h"
#include "../Nova/AI/JudgmentBase.h"

//	IdleNode‚É‘JˆÚ‚Å‚«‚é‚©”»’è
namespace DragonkinJudgment
{
	class IdleJudgment : public JudgmentBase
	{
	public:
		IdleJudgment(Dragonkin* owner) :JudgmentBase(owner) {};
		//	”»’è
		bool Judgment();

	};
}

//	SearchNode‚É‘JˆÚ‚Å‚«‚é‚©”»’è
namespace DragonkinJudgment
{
	class SearchJudgment :public JudgmentBase
	{
	public:
		SearchJudgment(Dragonkin* owner) :JudgmentBase(owner) {}
		//	”»’è
		bool Judgment();

	};
}

//	BattleNode‚É‘JˆÚ‚Å‚«‚é‚©”»’è
namespace DragonkinJudgment
{
	class BattleJudgment : public JudgmentBase
	{
	public:
		BattleJudgment(Dragonkin* owner) :JudgmentBase(owner) {};
		//	”»’è
		bool Judgment();

	};
}

//	SkillNode‚É‘JˆÚ‚Å‚«‚é‚©”»’è
namespace DragonkinJudgment
{
	class SkillJudgment : public JudgmentBase
	{
	public:
		SkillJudgment(Dragonkin* owner) :JudgmentBase(owner) {};
		//	”»’è
		bool Judgment();

	};
}

//	Escape‚É‘JˆÚ‚Å‚«‚é‚©”»’è
namespace DragonkinJudgment
{
	class EscapeJudgment : public JudgmentBase
	{
	public:
		EscapeJudgment(Dragonkin* owner) :JudgmentBase(owner) {};
		//	”»’è
		bool Judgment();
	
	};
}

//	Damage‚É‘JˆÚ‚Å‚«‚é‚©”»’è
namespace DragonkinJudgment
{
	class DamageJudgment :public JudgmentBase
	{
	public:
		DamageJudgment(Dragonkin* owner) :JudgmentBase(owner) {}
		//	”»’è
		bool Judgment();

	};
}

//	Death‚É‘JˆÚo‚é‚©”»’è
namespace DragonkinJudgment
{
	class DeathJudgment :public JudgmentBase
	{
	public:
		DeathJudgment(Dragonkin* owner) :JudgmentBase(owner) {}
		//	”»’è
		bool Judgment();

	};
}

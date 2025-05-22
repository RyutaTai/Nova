#include "DragonkinJudgment.h"

//	IdleJudgment
namespace DragonkinJudgment
{
	bool IdleJudgment::Judgment()
	{

		return true;
	}
}

//	SearchJudgment
namespace DragonkinJudgment
{
	bool SearchJudgment::Judgment()
	{
		//	プレイヤーを見つけていなければtrue
		if (owner_->SearchPlayer() == false)return true;
		return false;
	}
}

//	BattleJudgment
namespace DragonkinJudgment
{
	bool BattleJudgment::Judgment()
	{
		//	プレイヤーを見つけていたらtrue
		if (owner_->SearchPlayer())return true;
		return false;
	}
}

//	SkillJudgment
namespace DragonkinJudgment
{
	bool SkillJudgment::Judgment()
	{

		return true;
	}
}

//	EscapeJudgment
namespace DragonkinJudgment
{
	bool EscapeJudgment::Judgment()
	{

		return true;
	}
}

//	DamageJudgment
namespace DragonkinJudgment
{
	bool DamageJudgment::Judgment()
	{
		//	ダメージフラグが立っていたらtrue
		if (owner_->IsDamaged())
		{
			owner_->SetIsDamaged(false);
			return true;
		}

		return false;
	}
}

//	DeathJudgment
namespace DragonkinJudgment
{
	bool DeathJudgment::Judgment()
	{
		//	HPがなければtrue
		if (owner_->IsDead())
		{
			return true;
		}
		return false;
	}
}
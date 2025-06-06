#include "DragonkinAction.h"

#include "../../../../Nova/Others/MathHelper.h"
#include "../../Player/Player.h"

//	待機行動
namespace DragonkinAction
{
	ActionBase::State IdleAction::Run(const float& elapsedTime)
	{
		//	ダメージフラグ判定
		if (owner_->IsDamaged())
		{
			step_ = 0;
			return ActionBase::State::Failed;
		}

		//	死亡判定
		if (owner_->IsDead())
		{
			step_ = 0;
			return ActionBase::State::Complete;
		}

		switch (step_)
		{
		case 0:
			owner_->ResetRunTimer();
			owner_->PlayAnimation(static_cast<int>(Dragonkin::AnimationType::Idle01), false);
			step_++;
			break;
		case 1:
			//	タイマー更新
			owner_->UpdateRunTimer(elapsedTime);

			//	待機時間が過ぎた時
			if (owner_->GetRunTimer() >= idleTimer_)
			{
				owner_->SetRandomTargetPosition();
				step_ = 0;
				return ActionBase::State::Complete;
			}

			//	----- 旋回処理 -----
			owner_->Turn(elapsedTime);

			break;
		}
		return ActionBase::State::Run;
	}

	void IdleAction::DrawDebug()
	{
		if (ImGui::TreeNode("IdleAction"))
		{

			ImGui::TreePop();
		}
	}

}

//	索敵行動
namespace DragonkinAction
{
	ActionBase::State SearchAction::Run(const float& elapsedTime)
	{
		//	ダメージフラグ判定
		if (owner_->IsDamaged())
		{
			step_ = 0;
			return ActionBase::State::Failed;
		}

		//	死亡判定
		if (owner_->IsDead())
		{
			step_ = 0;
			return ActionBase::State::Complete;
		}

		switch (step_)
		{
		case 0:
			owner_->ResetRunTimer();
			step_++;
			break;
		case 1:
			//	プレイヤーを見つけた時
			if (owner_->SearchPlayer())
			{
				step_ = 0;
				return ActionBase::State::Complete;
			}

			break;
		}
		return ActionBase::State::Run;
	}

	void SearchAction::DrawDebug()
	{
		if (ImGui::TreeNode("SearchAction"))
		{

			ImGui::TreePop();
		}
	}

}

#pragma region ===== 攻撃 =====
//	攻撃待機
namespace DragonkinAction
{
	ActionBase::State AttackWaitAction::Run(const float& elapsedTime)
	{
		//	ダメージフラグ判定
		if (owner_->IsDamaged())
		{
			currentWaitTime_ = 0.0f;
			step_ = 0;
			return ActionBase::State::Failed;
		}

		//	死亡判定
		if (owner_->IsDead())
		{
			step_ = 0;
			return ActionBase::State::Complete;
		}

		switch (step_)
		{
		case 0:
			owner_->ResetRunTimer();
			currentWaitTime_ = 0.0f;
			step_++;
			break;
		case 1:
			// プレイヤーの位置と自分の位置から目標方向を計算
			DirectX::XMFLOAT3 playerPos = Player::Instance().GetTransform()->GetPosition();
			DirectX::XMFLOAT3 myPos = owner_->GetTransform()->GetPosition();
			DirectX::XMFLOAT3 toPlayer = { playerPos.x - myPos.x, 0.0f, playerPos.z - myPos.z };

			//	旋回を許可する
			owner_->SetIsTurnAction(true);

			//	旋回処理を実行
			owner_->Turn(elapsedTime);

			//	旋回が完了したかどうかの判定
			DirectX::XMVECTOR currentForward = DirectX::XMLoadFloat3(&owner_->GetTransform()->CalcForward());
			DirectX::XMVECTOR targetDir = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&toPlayer));

			float dot = DirectX::XMVectorGetX(DirectX::XMVector3Dot(currentForward, targetDir));

			//	旋回が完了した、または十分に近づいた場合
			if (dot >= cosf(angleThreshold_))
			{
				//	旋回後に少し待機時間を入れる
				currentWaitTime_ += elapsedTime;
				if (currentWaitTime_ >= waitTimer_)
				{
					currentWaitTime_ = 0.0f;			//	リセット
					step_ = 0;
					return ActionBase::State::Complete; //	アクション完了
				}
			}
			
			break;
		}
		return ActionBase::State::Run; //	旋回中または待機中
	}

	void AttackWaitAction::DrawDebug()
	{
		if (ImGui::TreeNode("AttackWaitAction"))
		{
			ImGui::DragFloat("Angle Threshold (Degrees)", &angleThreshold_, 0.01f, 0.0f, 180.0f);
			ImGui::DragFloat("Wait Timer", &waitTimer_, 0.01f, 0.0f, 5.0f);
			ImGui::TreePop();
		}
	}
}

#pragma region ----- 通常攻撃 -----
//	通常殴打
namespace DragonkinAction
{
	ActionBase::State AttackPunchAction::Run(const float& elapsedTime)
	{
		//	ダメージフラグ判定
		if (owner_->IsDamaged())
		{
			step_ = 0;
			return ActionBase::State::Failed;
		}

		//	死亡判定
		if (owner_->IsDead())
		{
			step_ = 0;
			return ActionBase::State::Complete;
		}

		switch (step_)
		{
		case 0:
			owner_->ResetRunTimer();
			owner_->PlayAnimation(static_cast<int>(Dragonkin::AnimationType::AttackPunch), false);
			//	判定を取る区間を設定
			animJudgeTime_.SetRange(0.42f, 0.438f);

			//	攻撃中は旋回しない
			owner_->SetIsTurnAction(false);

			step_++;
			break;
		case 1:
			//	プレイヤーとの当たり判定
			float currentAnimationSeconds = owner_->GetCurrentAnimationSeconds();	//	アニメーション再生時間
			if (animJudgeTime_.IsWithinRange(currentAnimationSeconds))
			{
				owner_->GetAttackDetectionData("Hand_R")->SetIsActive(true);
				owner_->GetAttackDetectionData("Hand_R_1")->SetIsActive(true);

				//	攻撃中は押し出し判定しない
				Player::Instance().SetIsActiveCollisionDetection(false);

			}
			else
			{
				owner_->GetAttackDetectionData("Hand_R")->SetIsActive(false);
				owner_->GetAttackDetectionData("Hand_R_1")->SetIsActive(false);

				//	押し出し判定をオンにする
				Player::Instance().SetIsActiveCollisionDetection(true);
			}

			//	アニメーション再生が終わったら終了
			if (owner_->IsPlayAnimation() == false)
			{
				step_ = 0;
				return ActionBase::State::Complete;
			}

			break;
		}
		return ActionBase::State::Run;
	}

	void AttackPunchAction::DrawDebug()
	{
		if (ImGui::TreeNode("AttackPunchAction"))
		{

			ImGui::TreePop();
		}
	}

}

//	通常キック
namespace DragonkinAction
{
	ActionBase::State AttackKickAction::Run(const float& elapsedTime)
	{
		//	ダメージフラグ判定
		if (owner_->IsDamaged())
		{
			step_ = 0;
			return ActionBase::State::Failed;
		}

		//	死亡判定
		if (owner_->IsDead())
		{
			step_ = 0;
			return ActionBase::State::Complete;
		}

		switch (step_)
		{
		case 0:
			owner_->ResetRunTimer();
			owner_->PlayAnimation(static_cast<int>(Dragonkin::AnimationType::AttackKick), false);
			//	判定を取る区間を設定
			animJudgeTime_.SetRange(0.52f, 0.6f);

			//	攻撃中は旋回しない
			owner_->SetIsTurnAction(false);
			
			step_++;
			break;
		case 1:
			//	プレイヤーとの当たり判定
			float currentAnimationSeconds = owner_->GetCurrentAnimationSeconds();	//	アニメーション再生時間
			if (animJudgeTime_.IsWithinRange(currentAnimationSeconds))
			{
				owner_->GetAttackDetectionData("Foot_L")->SetIsActive(true);
				owner_->GetAttackDetectionData("calf_l")->SetIsActive(true);

				//	攻撃中は押し出し判定しない
				Player::Instance().SetIsActiveCollisionDetection(false);

			}
			else
			{
				owner_->GetAttackDetectionData("Foot_L")->SetIsActive(false);
				owner_->GetAttackDetectionData("calf_l")->SetIsActive(false);

				//	押し出し判定をオンにする
				Player::Instance().SetIsActiveCollisionDetection(true);

			}

			//	アニメーション再生が終わったら終了
			if (owner_->IsPlayAnimation() == false)
			{
				step_ = 0;
				return ActionBase::State::Complete;
			}

			break;
		}
		return ActionBase::State::Run;
	}

	void AttackKickAction::DrawDebug()
	{
		if (ImGui::TreeNode("AttackKickAction"))
		{

			ImGui::TreePop();
		}
	}

}

//	通常翼攻撃
namespace DragonkinAction
{
	ActionBase::State AttackWingAction::Run(const float& elapsedTime)
	{
		//	ダメージフラグ判定
		if (owner_->IsDamaged())
		{
			step_ = 0;
			return ActionBase::State::Failed;
		}

		//	死亡判定
		if (owner_->IsDead())
		{
			step_ = 0;
			return ActionBase::State::Complete;
		}

		switch (step_)
		{
		case 0:
			owner_->ResetRunTimer();
			owner_->PlayAnimation(static_cast<int>(Dragonkin::AnimationType::AttackWing), false);
			//	判定を取る区間を設定
			animJudgeTime_.SetRange(0.34f, 0.41f);

			//	攻撃中は旋回しない
			owner_->SetIsTurnAction(false);
			
			step_++;
			break;
		case 1:
			//	プレイヤーとの当たり判定
			float currentAnimationSeconds = owner_->GetCurrentAnimationSeconds();	//	アニメーション再生時間
			if (animJudgeTime_.IsWithinRange(currentAnimationSeconds))
			{
				owner_->GetAttackDetectionData("Wing_L03")->SetIsActive(true);
				owner_->GetAttackDetectionData("Wing_L04")->SetIsActive(true);
				owner_->GetAttackDetectionData("Wing_L05")->SetIsActive(true);
				owner_->GetAttackDetectionData("Wing_L05_1")->SetIsActive(true);
				owner_->GetAttackDetectionData("Wing_L05_2")->SetIsActive(true);
				owner_->GetAttackDetectionData("Wing_L06")->SetIsActive(true);
				owner_->GetAttackDetectionData("Wing_L06_1")->SetIsActive(true);
				owner_->GetAttackDetectionData("Wing_L06_2")->SetIsActive(true);
				owner_->GetAttackDetectionData("Wing_L06_3")->SetIsActive(true);
				owner_->GetAttackDetectionData("Wing_L06_4")->SetIsActive(true);
				owner_->GetAttackDetectionData("Wing_L08")->SetIsActive(true);
				owner_->GetAttackDetectionData("Wing_L08_1")->SetIsActive(true);
				owner_->GetAttackDetectionData("Wing_L08_2")->SetIsActive(true);
				owner_->GetAttackDetectionData("Wing_L09")->SetIsActive(true);
				owner_->GetAttackDetectionData("Wing_L09_1")->SetIsActive(true);
				owner_->GetAttackDetectionData("Wing_L10")->SetIsActive(true);
				owner_->GetAttackDetectionData("Wing_L10_1")->SetIsActive(true);
				owner_->GetAttackDetectionData("Wing_L10_2")->SetIsActive(true);
				owner_->GetAttackDetectionData("Wing_L10_3")->SetIsActive(true);
				owner_->GetAttackDetectionData("Wing_L10_4")->SetIsActive(true);
				owner_->GetAttackDetectionData("Wing_L10_5")->SetIsActive(true);
				owner_->GetAttackDetectionData("Wing_L10_6")->SetIsActive(true);

				//	攻撃中は押し出し判定しない
				Player::Instance().SetIsActiveCollisionDetection(false);

			}
			else
			{
				owner_->GetAttackDetectionData("Wing_L03")->SetIsActive(false);
				owner_->GetAttackDetectionData("Wing_L04")->SetIsActive(false);
				owner_->GetAttackDetectionData("Wing_L05")->SetIsActive(false);
				owner_->GetAttackDetectionData("Wing_L05_1")->SetIsActive(false);
				owner_->GetAttackDetectionData("Wing_L05_2")->SetIsActive(false);
				owner_->GetAttackDetectionData("Wing_L06")->SetIsActive(false);
				owner_->GetAttackDetectionData("Wing_L06_1")->SetIsActive(false);
				owner_->GetAttackDetectionData("Wing_L06_2")->SetIsActive(false);
				owner_->GetAttackDetectionData("Wing_L06_3")->SetIsActive(false);
				owner_->GetAttackDetectionData("Wing_L06_4")->SetIsActive(false);
				owner_->GetAttackDetectionData("Wing_L08")->SetIsActive(false);
				owner_->GetAttackDetectionData("Wing_L08_1")->SetIsActive(false);
				owner_->GetAttackDetectionData("Wing_L08_2")->SetIsActive(false);
				owner_->GetAttackDetectionData("Wing_L09")->SetIsActive(false);
				owner_->GetAttackDetectionData("Wing_L09_1")->SetIsActive(false);
				owner_->GetAttackDetectionData("Wing_L10")->SetIsActive(false);
				owner_->GetAttackDetectionData("Wing_L10_1")->SetIsActive(false);
				owner_->GetAttackDetectionData("Wing_L10_2")->SetIsActive(false);
				owner_->GetAttackDetectionData("Wing_L10_3")->SetIsActive(false);
				owner_->GetAttackDetectionData("Wing_L10_4")->SetIsActive(false);
				owner_->GetAttackDetectionData("Wing_L10_5")->SetIsActive(false);
				owner_->GetAttackDetectionData("Wing_L10_6")->SetIsActive(false);

				//	押し出し判定をオンにする
				Player::Instance().SetIsActiveCollisionDetection(true);

			}

			//	アニメーション再生が終わったら終了
			if (owner_->IsPlayAnimation() == false)
			{
				step_ = 0;
				return ActionBase::State::Complete;
			}

			break;
		}
		return ActionBase::State::Run;
	}

	void AttackWingAction::DrawDebug()
	{
		if (ImGui::TreeNode("AttackWingAction"))
		{

			ImGui::TreePop();
		}
	}

}
#pragma endregion ----- 攻撃 -----

#pragma region ----- スキル攻撃 -----
//	スキル攻撃行動
namespace DragonkinAction
{
	ActionBase::State SkillAction::Run(const float& elapsedTime)
	{
		//	ダメージフラグ判定
		if (owner_->IsDamaged())
		{
			step_ = 0;
			return ActionBase::State::Failed;
		}

		//	死亡判定
		if (owner_->IsDead())
		{
			step_ = 0;
			return ActionBase::State::Complete;
		}

		switch (step_)
		{
		case 0:
			owner_->ResetRunTimer();
			step_++;
			break;
		case 1:
			return ActionBase::State::Complete;
			break;
		}
		return ActionBase::State::Run;
	}

	void SkillAction::DrawDebug()
	{
		if (ImGui::TreeNode("SkillAction"))
		{

			ImGui::TreePop();
		}
	}
}
#pragma endregion ----- スキル攻撃 -----
#pragma endregion ===== 攻撃 =====

//	追跡行動
namespace DragonkinAction
{
	ActionBase::State PursuitAction::Run(const float& elapsedTime)
	{
		//	ダメージフラグ判定
		if (owner_->IsDamaged())
		{
			step_ = 0;
			return ActionBase::State::Failed;
		}

		//	死亡判定
		if (owner_->IsDead())
		{
			step_ = 0;
			return ActionBase::State::Complete;
		}

		switch (step_)
		{
		case 0:
			owner_->ResetRunTimer();
			step_++;
			break;
		case 1:
			return ActionBase::State::Complete;
			break;
		}
		return ActionBase::State::Run;
	}

	void PursuitAction::DrawDebug()
	{
		if (ImGui::TreeNode("PursuitAction"))
		{

			ImGui::TreePop();
		}
	}

}

//	逃走行動
namespace DragonkinAction
{
	ActionBase::State LeaveAction::Run(const float& elapsedTime)
	{
		//	ダメージフラグ判定
		if (owner_->IsDamaged())
		{
			step_ = 0;
			return ActionBase::State::Failed;
		}

		//	死亡判定
		if (owner_->IsDead())
		{
			step_ = 0;
			return ActionBase::State::Complete;
		}

		switch (step_)
		{
		case 0:
			owner_->ResetRunTimer();
			step_++;
			break;
		case 1:
			return ActionBase::State::Complete;
			break;
		}
		return ActionBase::State::Run;
	}

	void LeaveAction::DrawDebug()
	{
		if (ImGui::TreeNode("LeaveAction"))
		{

			ImGui::TreePop();
		}
	}

}

//	回復行動
namespace DragonkinAction
{
	ActionBase::State RecoverAction::Run(const float& elapsedTime)
	{
		//	ダメージフラグ判定
		if (owner_->IsDamaged())
		{
			step_ = 0;
			return ActionBase::State::Failed;
		}

		//	死亡判定
		if (owner_->IsDead())
		{
			step_ = 0;
			return ActionBase::State::Complete;
		}

		switch (step_)
		{
		case 0:
			owner_->ResetRunTimer();
			step_++;
			break;
		case 1:
			return ActionBase::State::Complete;
			break;
		}
		return ActionBase::State::Run;
	}

	void RecoverAction::DrawDebug()
	{
		if (ImGui::TreeNode("RecoverAction"))
		{

			ImGui::TreePop();
		}
	}
}

//	ダメージ行動
namespace DragonkinAction
{
	ActionBase::State DamageAction::Run(const float& elapsedTime)
	{
		//	死亡判定
		if (owner_->IsDead())
		{
			step_ = 0;
			return ActionBase::State::Complete;
		}

		switch (step_)
		{
		case 0:
			owner_->ResetRunTimer();
			owner_->PlayAnimation(static_cast<int>(Dragonkin::AnimationType::DamageHit02), false, 0.03f, 1.0f, startFrame_, endFrame_);
			step_++;
			break;
		case 1:
			//	アニメーション再生が終わったら終了
			if (owner_->IsPlayAnimation() == false)
			{
				step_ = 0;
				return ActionBase::State::Complete;
			}
			break;
		}
		return ActionBase::State::Run;
	}

	void DamageAction::DrawDebug()
	{
		if (ImGui::TreeNode("DamageAction"))
		{
			//	----- アニメーション -----
			ImGui::DragFloat("StartFrame", &startFrame_, 0.01f);
			ImGui::DragFloat("EndFrame", &endFrame_, 0.01f);

			ImGui::TreePop();
		}
	}
}

//	死亡行動
namespace DragonkinAction
{
	ActionBase::State DeathAction::Run(const float& elapsedTIme)
	{
		switch (step_)
		{
		case 0:
			owner_->ResetRunTimer();
			owner_->PlayAnimation(static_cast<int>(Dragonkin::AnimationType::DmageDieDown), false, 0.2f);
			step_++;
			break;
		case 1:
			//	アニメーション再生が終わったら終了
			if (owner_->IsPlayAnimation() == false)
			{
				step_ = 0;
				owner_->SetIsDead(true);
				owner_->Destroy();
				return ActionBase::State::Complete;
			}
			break;
		}
		return ActionBase::State::Run;
	}

	void DeathAction::DrawDebug()
	{
		if (ImGui::TreeNode("DeathAction"))
		{

			ImGui::TreePop();
		}
	}
}
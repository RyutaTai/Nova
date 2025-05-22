#include "DroneState.h"

//	待機ステート
namespace DroneState
{
	void IdleState::Initialize()
	{
		
	}

	void IdleState::Update(const float& elapsedTime)
	{
		//	待機時間が経過したら索敵ステートへ遷移
		if (stateElapsedTime_ > IdleTime_)
		{
			owner_->ChangeState(Drone::StateType::Search);
			return;
		}

		//	ダメージステートへ遷移
		owner_->ChangeDamageState();

	}

	void IdleState::Finalize()
	{

	}

	void IdleState::DrawDebug()
	{
		if (ImGui::TreeNode("IdleState"))
		{
			ImGui::DragFloat("ElapsedTime", &stateElapsedTime_, 0.1f);
			ImGui::DragFloat("IdleTime", &IdleTime_, 0.1f);

			ImGui::TreePop();
		}

	}

}

//	サーチステート
namespace DroneState
{
	void SearchState::Initialize()
	{

	}

	void SearchState::Update(const float& elapsedTime)
	{
		//	プレイヤーを見つけたら移動ステートへ遷移
		if (owner_->SearchPlayer())
		{
			owner_->ChangeState(Drone::StateType::Pursuit);
			return;
		}

		//	ダメージステートへ遷移
		owner_->ChangeDamageState();

	}

	void SearchState::Finalize()
	{

	}

	void SearchState::DrawDebug()
	{
		if (ImGui::TreeNode("SearchState"))
		{
			ImGui::DragFloat("ElapsedTIme", &stateElapsedTime_, 0.1f);

			ImGui::TreePop();
		}
	}

}

//	移動ステート
namespace DroneState
{
	void MoveState::Initialize()
	{

	}

	void MoveState::Update(const float& elapsedTime)
	{
		//	ダメージステートへ遷移
		owner_->ChangeDamageState();
	}

	void MoveState::Finalize()
	{
		owner_->SetMoveVec({});	//	移動をやめる
	}

	void MoveState::DrawDebug()
	{
		if (ImGui::TreeNode("MoveState"))
		{
			ImGui::DragFloat("ElapsedTime", &stateElapsedTime_, 0.1f);

			ImGui::TreePop();
		}
	}

}

//	追跡ステート
namespace DroneState
{
	void PursuitState::Initialize()
	{

	}

	void PursuitState::Update(const float& elapsedTime)
	{
		//	----- 追跡処理 -----
		//	プレイヤーが索敵範囲外なら待機ステートへ遷移
		if (owner_->CalcDistanceToTarget() > owner_->GetSearchRange())
		{
			owner_->SetMoveVec({});							//	移動をやめる
			owner_->ChangeState(Drone::StateType::Idle);	//	待機ステートへ遷移
			return;
		}

		//	プレイヤーが射程範囲内なら攻撃ステートへ遷移
		if (owner_->CalcDistanceToTarget() < owner_->GetLaunchRange())
		{
			owner_->SetMoveVec({});							//	移動をやめる
			owner_->ChangeState(Drone::StateType::Attack);	//	攻撃ステートへ遷移
			return;
		}

		//	ダメージステートへ遷移
		owner_->ChangeDamageState();

		//	プレイヤーが索敵範囲内かつ、射程範囲外なら近づく
		owner_->ApproachingTarget(elapsedTime);

	}

	void PursuitState::Finalize()
	{
		owner_->SetMoveVec({});	//	移動をやめる
	}

	void PursuitState::DrawDebug()
	{
		if (ImGui::TreeNode("PursuitState"))
		{
			ImGui::DragFloat("ElapsedTime", &stateElapsedTime_, 0.1f);

			ImGui::TreePop();
		}
	}
}

//	攻撃ステート
namespace DroneState
{
	void AttackState::Initialize()
	{
		//	最初はすぐに発射する
		//owner_->SetLaunchTimer(owner_->GetLaunchInterval());
		//owner_->ResetLaunchTimer();
		owner_->SetLaunchTimer(initLaunchTimer_);
	}

	void AttackState::Update(const float& elapsedTime)
	{
		//	----- 弾丸処理 -----
		owner_->LaunchBullet(elapsedTime);

		//	プレイヤーが射程範囲外に行ったら待機ステートへ遷移
		if (owner_->CalcDistanceToTarget() >= owner_->GetLaunchRange())
		{
			owner_->ChangeState(Drone::StateType::Idle);
		}

		//	ダメージステートへ遷移
		owner_->ChangeDamageState();

	}

	void AttackState::Finalize()
	{

	}

	void AttackState::DrawDebug()
	{
		if (ImGui::TreeNode("AttackState"))
		{
			ImGui::DragFloat("ElapsedTime", &stateElapsedTime_, 0.1f);
			ImGui::DragFloat("InitLaunchTimer", &initLaunchTimer_, 0.01f);

			ImGui::TreePop();
		}
	}

}

//	回避ステート
namespace DroneState
{
	void AvoidanceState::Initialize()
	{

	}

	void AvoidanceState::Update(const float& elapsedTime)
	{
		//	ダメージステートへ遷移
		owner_->ChangeDamageState();
	}

	void AvoidanceState::Finalize()
	{

	}

	void AvoidanceState::DrawDebug()
	{
		if (ImGui::TreeNode("AvoidanceState"))
		{
			ImGui::DragFloat("ElapsedTime", &stateElapsedTime_, 0.1f);

			ImGui::TreePop();
		}
	}

}

//	ダメージステート
namespace DroneState
{
	void DamageState::Initialize()
	{
		//	ダメージフラグリセット
		owner_->SetIsDamaged(false);

		//	ダメージを受けたら発射タイマーリセット
		owner_->ResetLaunchTimer();

	}

	void DamageState::Update(const float& elapsedTime)
	{
		owner_->ChangeState(Drone::StateType::Idle);
	}

	void DamageState::Finalize()
	{

	}

	void DamageState::DrawDebug()
	{
		if (ImGui::TreeNode("DamageState"))
		{
			ImGui::DragFloat("ElapsedTime", &stateElapsedTime_, 0.1f);

			ImGui::TreePop();
		}
	}

}

//	死亡ステート
namespace DroneState
{
	void DeathState::Initialize()
	{
		//	タイマーリセット
		destroyTimer_ = 0.0f;
	}

	void DeathState::Update(const float& elapsedTime)
	{
		//	タイマー更新
		destroyTimer_ += elapsedTime;

		//	破棄処理
		if (destroyTimer_ > destroyDuration_)
		{
			owner_->Destroy();
		}
	}

	void DeathState::Finalize()
	{

	}

	void DeathState::DrawDebug()
	{
		if (ImGui::TreeNode("DeathState"))
		{
			ImGui::DragFloat("ElapsedTime", &stateElapsedTime_, 0.1f);

			ImGui::DragFloat("DestroyTimer", &destroyTimer_, 0.01f);
			ImGui::DragFloat("DestroyDuration", &destroyDuration_, 0.01f);

			ImGui::TreePop();
		}
	}

}
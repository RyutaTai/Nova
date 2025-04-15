#include "PlayerState.h"

#include <algorithm>

#include "Bullet.h"
#include "BulletManager.h"
#include "EnemyManager.h"
#include "Rhythm.h"
#include "../Nova/Debug/DebugRenderer.h"
#include "../Nova/Graphics/Graphics.h"
#include "../Nova/Collision/Collision.h"
#include "../Nova/Others/MathHelper.h"
#include "../Nova/Input/GamePad.h"
#include "../Nova/Input/Input.h"
#include "../Nova/Audio/AudioManager.h"
#include "../Nova/Scenes/SceneManager.h"
#include "../Nova/Collision/CollisionManager.h"

//	待機ステート
namespace PlayerState
{
	void IdleState::Initialize()
	{
		//	アニメーションセット
		//owner_->PlayAnimation(Player::AnimationType::Idle, true, 0.2f);
		owner_->PlayAnimation(Player::AnimationType::Idle, true, blendAnimTime_);
		owner_->SetAnimationSpeed(1.0f);
	}

	void IdleState::Update(const float& elapsedTime)
	{
		//	ステートへ遷移
		DetermineStateTransition(elapsedTime);

	}

	//	ステート遷移を判断する
	void IdleState::DetermineStateTransition(const float& elapsedTime)
	{
		//	移動入力があれば、移動ステートへ遷移
		if (owner_->InputMove(elapsedTime))
		{
			owner_->ChangeState(Player::StateType::Move);
			return;
		}

		//	攻撃ステートへ遷移
		if(Input::Instance().GetGamePad().GetButtonDown() & GamePad::BTN_A/*Zキー*/)
		{
			owner_->ChangeState(Player::StateType::ComboOne1);

			//	リズム判定処理
			Rhythm::Instance().GetJudgmentType(Rhythm::Instance().GetCurrentMidiTime());
			return;
		}

		//	回避ステートへ遷移
		owner_->ChangeDodgeState();

	}

	void IdleState::Finalize()
	{
		owner_->SetAnimationSpeed(1.0f);
	}

	void IdleState::DrawDebug()
	{
		if (ImGui::TreeNode("IdleState"))
		{
			ImGui::DragFloat("StateElapsedTime", &stateElapsedTime_, 0.1f);
			ImGui::DragFloat("BlendAnimTime", &blendAnimTime_, 0.001f);

			ImGui::TreePop();
		}
	}
}

//	移動ステート
namespace PlayerState
{
	void MoveState::Initialize()
	{
		//	アニメーションセット
		owner_->PlayAnimation(Player::AnimationType::Run, true, blendAnimTime_);
		owner_->SetAnimationSpeed(1.2f);

		//	移動速度を設定
		owner_->SetMoveSpeed(moveSpeed_);

		//	足音タイマーリセット
		footStepsTimer_ = 0.0f;

	}

	void MoveState::Update(const float& elapsedTime)
	{
		//	足音再生
		PlayFootstepsSE(elapsedTime);

		//	ステート遷移を判断
		DetermineStateTransition(elapsedTime);
		
	}

	//	ステート遷移を判断
	void MoveState::DetermineStateTransition(const float& elapsedTime)
	{
		//	移動入力がなくなったら待機ステートへ遷移
		if (owner_->InputMove(elapsedTime) == false)
		{
			//	待機ステートへ遷移
			owner_->ChangeState(Player::StateType::Idle);
			return;
		}

		//	攻撃ステートへ遷移
		if (Input::Instance().GetGamePad().GetButtonDown() & GamePad::BTN_A/*Zキー*/)
		{
			owner_->ChangeState(Player::StateType::ComboOne1);

			//	リズム判定処理
			Rhythm::Instance().GetJudgmentType(Rhythm::Instance().GetCurrentMidiTime());
			return;
		}

		//	回避ステートへ遷移
		owner_->ChangeDodgeState();

	}

	//	足音SE再生
	void MoveState::PlayFootstepsSE(const float& elapsedTime)
	{
		//	ピッチを0.4～0.6の間でランダムに決めている
#if 1
		srand(static_cast<unsigned int>(time(NULL)));
		float pitch = (rand() % 20 + 40) / 100.0f;
#else
		float pitch = Mathf::RandomRange(0.4f, 0.6f);	//	重い
#endif
		//	再生間隔に達していて、SEの再生も終わっていたら再生する
		if (footStepsTimer_ > playFootstepsInterval_)
		{
			footStepsTimer_ = 0.0f;
			AudioManager::Instance().GetAudioResource("PlayerFootsteps")->SetPitch(pitch);	//	ピッチ設定
			AudioManager::Instance().GetAudioResource("PlayerFootsteps")->Play(false);		//	再生

		}

		//	足音再生間隔更新
		footStepsTimer_ += elapsedTime;

	}

	void MoveState::Finalize()
	{
		owner_->ResetMoveSpeed();
		owner_->SetAnimationSpeed(1.0f);

		//	足音停止
		//AudioManager::Instance().GetAudioResource("PlayerFootsteps")->Stop();

		//	足音タイマーリセット
		footStepsTimer_ = 0.0f;

	}

	void MoveState::DrawDebug()
	{
		if (ImGui::TreeNode("MoveState"))
		{
			//	----- 経過時間 -----
			ImGui::DragFloat("StateElapsedTime", &stateElapsedTime_, 0.1f);

			//	----- 移動速度 -----
			ImGui::DragFloat("MoveSpeed", &moveSpeed_, 0.01f);
			
			//	----- アニメーション再生 -----
			ImGui::DragFloat("BlendTime", &blendAnimTime_, 0.001f);

			//	----- 足音SE -----
			ImGui::DragFloat("PlayFootstepsInterval", &playFootstepsInterval_, 0.01f);	//	足音SE再生間隔
			ImGui::DragFloat("FootstepsTimer", &footStepsTimer_, 0.01f);				//	足音SE再生間隔タイマー

			bool isPlaying = AudioManager::Instance().GetAudioResource("PlayerFootsteps")->IsPlaying();	//	再生中フラグ
			ImGui::Checkbox("IsPlaying", &isPlaying);

			ImGui::TreePop();
		}
	}
}

//	コンボ01_1(右パンチ)
namespace PlayerState
{
	void ComboOne1::Initialize()
	{
		//	アニメーションセット
		owner_->PlayAnimation(Player::AnimationType::ComboOne1, false, 0.0f);
		owner_->SetAnimationSpeed(1.0f);

		//	ルートモーション
		//owner_->SetUseRootMotion(true);

		//	判定時間セット
		animJudgeTime_.SetJudgeTime(0.180f, 0.38f);			//	アニメーション判定区間
		//acceptInputFrame_ = 10.0f;						//	先行入力受付フレーム
		cancellationTime_.SetJudgeTime(0.3f, 1.16f);		//	キャンセル可能時間
		cancellationTime_.SetName("CancellationTime");

		//	アニメーション速度変化区間セット
		animSpeedChangeInterval_[0].SetJudgeTime(0.0f, 0.32f);		//	パンチ前
		animSpeedChangeInterval_[1].SetJudgeTime(0.32f, 0.67f);		//	パンチ
		animSpeedChangeInterval_[2].SetJudgeTime(0.67f, 1.167f);	//	パンチ後

		//	ステート経過時間初期化
		stateElapsedTime_ = 0.0f;

		//	プレイヤーの攻撃判定を無効にする
		owner_->SetAllAttackDetectionActiveFlag(false);
		owner_->SetAttackHit(false);

		//	攻撃中は押し出し判定しない
		owner_->SetIsActiveCollisionDetection(false);

		//	プレイヤーのコンボ数を初期化
		owner_->ResetComboCount();

	}

	void ComboOne1::Update(const float& elapsedTime)
	{
		//	アニメーション速度更新
		UpdateAnimationSpeed();			

		//	プレイヤーの攻撃判定を有効にする
		float currentAnimationSeconds = owner_->GetCurrentAnimationSeconds();	//	アニメーション再生時間
		if (animJudgeTime_.IsJudgeFlag(currentAnimationSeconds))
		{
			owner_->GetAttackDetectionData("RightPunch").SetIsActive(true);
		}

		//	次のステートへ遷移
		DetermineStateTransition(elapsedTime);
		
	}

	//	ステート遷移を判断
	void ComboOne1::DetermineStateTransition(const float& elapsedTime)
	{
		//	次のステートへの遷移
		if (JudgeInput(cancellationTime_))
		{
			//	リズム判定処理(missならreturn)
			if (Rhythm::Instance().GetJudgmentType(Rhythm::Instance().GetCurrentMidiTime()) == Rhythm::JudgmentType::Miss)
				return;

			//	miss以外なら次のステートへ遷移
			owner_->ChangeState(Player::StateType::ComboOne2);
			return;
		}
		if (owner_->IsPlayAnimation() == false)
		{
			owner_->ChangeState(Player::StateType::Idle);
			return;
		}

		//	回避ステートへ遷移
		owner_->ChangeDodgeState();

	}

	bool ComboOne1::JudgeInput(const JudgeTime& cancellationTime)
	{
		//	オートコンボがオンなら入力判定をtrueにする
		if (owner_->IsAutoCombo())
		{
			return true;
		}

		//if (cancellationTime.IsJudgeFlag(stateElapsedTime_) == false)return false;

		if(Input::Instance().GetGamePad().GetButtonDown() & GamePad::BTN_A/*Zキー*/)
		{
			return true;
		}

		return false;
	}

	bool ComboOne1::JudgeInputCommand(const JudgeTime& cancellationTime, const Command& command)
	{
		//	オートコンボがオンなら入力判定をtrueにする
		if (owner_->IsAutoCombo())
		{
			return true;
		}

		if (cancellationTime.IsJudgeFlag(stateElapsedTime_) == false)return false;
		
		if (Input::Instance().CommandConfirm(command, acceptInputFrame_))
		{
			return true;
		}

		return false;
	}

	//	アニメーション速度の微調整
	void ComboOne1::UpdateAnimationSpeed()
	{
		float currentAnimationSeconds = owner_->GetCurrentAnimationSeconds();	//	アニメーション再生時間

		if(animSpeedChangeInterval_[0].IsJudgeFlag(currentAnimationSeconds))
			owner_->SetAnimationSpeed(2.5f);
		else if(animSpeedChangeInterval_[1].IsJudgeFlag(currentAnimationSeconds))
			owner_->SetAnimationSpeed(1.5f);
		else if(animSpeedChangeInterval_[2].IsJudgeFlag(currentAnimationSeconds))
			owner_->SetAnimationSpeed(1.1f);
	}

	void ComboOne1::Finalize()
	{
		owner_->SetUseRootMotion(false);
		
		owner_->SetAnimationSpeed(1.0f);

		//	プレイヤーの攻撃判定を無効にする
		owner_->SetAllAttackDetectionActiveFlag(false);
		//	プレイヤーの押し出し判定を有効化
		owner_->SetIsActiveCollisionDetection(true);

	}

	void ComboOne1::DrawDebug()
	{
		if (ImGui::TreeNode("ComboOne1 State"))
		{
			ImGui::DragFloat("StateElapsedTime", &stateElapsedTime_);	//	ステート経過時間
			ImGui::DragFloat("AcceptFrame", &acceptInputFrame_);		//	入力受付フレーム
			
			//	キャンセル可能時間
			cancellationTime_.DrawDebug();

			ImGui::TreePop();
		}
	}

}

//	コンボ01_2
namespace PlayerState
{
	void ComboOne2::Initialize()
	{
		//	アニメーションセット
		owner_->PlayAnimation(Player::AnimationType::ComboOne2, false, 0.0f);
		owner_->SetAnimationSpeed(1.0f);

		//	ルートモーション
		//owner_->SetUseRootMotion(true);

		//	判定時間セット
		animJudgeTime_[0].SetJudgeTime(0.21f, 0.25f);	//	アニメーション再生中に当たっているか判定(アニメーション再生時間をもとに判定)
		animJudgeTime_[1].SetJudgeTime(0.29f, 0.63f);
		acceptInputFrame_ = 10.0f;
		cancellationTime_.SetJudgeTime(0.64f, 1.617f);	//	キャンセル可能時間
		cancellationTime_.SetName("CancellationTime");

		//	アニメーション再生速度変化区間セット
		animSpeedChangeInterval_[0].SetJudgeTime(0.0f, 0.23f);		//	左パンチ出すまで
		animSpeedChangeInterval_[1].SetJudgeTime(0.24f, 0.55f);		//	左パンチからアッパー
		animSpeedChangeInterval_[2].SetJudgeTime(0.56f, 1.3f);		//	アッパーから構え
		animSpeedChangeInterval_[3].SetJudgeTime(1.1f, 1.617f);		//	構えから待機に戻る

		//	ステート経過時間初期化
		stateElapsedTime_ = 0.0f;

		//	プレイヤーの攻撃判定を無効にする
		owner_->SetAllAttackDetectionActiveFlag(false);
		owner_->SetAttackHit(false);
		//	攻撃中は押し出し判定しない
		owner_->SetIsActiveCollisionDetection(false);
	}

	void ComboOne2::Update(const float& elapsedTime)
	{
		//	アニメーション速度更新
		UpdateAnimationSpeed();

		//	当たり判定処理(アニメーションが再生されたら再生時間をもとに判定する)
		//	一撃目
		float currentAnimationSeconds = owner_->GetCurrentAnimationSeconds();	//	アニメーション再生時間
		if (animJudgeTime_[0].IsJudgeFlag(currentAnimationSeconds))
			owner_->GetAttackDetectionData("LeftPunch").SetIsActive(true);

		//	一撃目のアニメーションが終わったらヒットフラグをオフにする
		if (animJudgeTime_[0].GetMaxTime() < currentAnimationSeconds && currentAnimationSeconds < animJudgeTime_[1].GetMinTime())
			owner_->SetAttackHit(false);

		//	二撃目
		if (animJudgeTime_[1].IsJudgeFlag(currentAnimationSeconds))
			owner_->GetAttackDetectionData("RightPunch").SetIsActive(true);

		//	次のステートへ遷移
		DetermineStateTransition(elapsedTime);

	}

	//	ステート遷移を判断
	void ComboOne2::DetermineStateTransition(const float& elapsedTime)
	{
		//	次のステートへ遷移
		if (JudgeInput(cancellationTime_))	//	入力判定がtrueならコンボを進める
		{
			//	リズム判定処理(missならreturn)
			if (Rhythm::Instance().GetJudgmentType(Rhythm::Instance().GetCurrentMidiTime()) == Rhythm::JudgmentType::Miss)
				return;

			//	次のステートへ遷移
			owner_->ChangeState(Player::StateType::ComboOne3);
			return;
		}

		//	アニメーション再生が終わったら待機へ遷移
		if (owner_->IsPlayAnimation() == false)
		{
			owner_->ChangeState(Player::StateType::Idle);
			return;
		}

		//	回避ステートへ遷移
		owner_->ChangeDodgeState();

	}

	bool ComboOne2::JudgeInput(const JudgeTime& cancellationTime)
	{
		//	オートコンボがオンなら入力判定をtrueにする
		if (owner_->IsAutoCombo())
		{
			return true;
		}

		//if (cancellationTime.IsJudgeFlag(stateElapsedTime_) == false)return false;

		if(Input::Instance().GetGamePad().GetButtonDown() & GamePad::BTN_A/*Zキー*/)
		{
			return true;
		}

		return false;

	}

	bool ComboOne2::JudgeInputCommand(const JudgeTime& cancellationTime, const Command& command)
	{
		//	オートコンボがオンなら入力判定をtrueにする
		if (owner_->IsAutoCombo())
		{
			return true;
		}

		if (cancellationTime.IsJudgeFlag(stateElapsedTime_) == false)return false;

		if (Input::Instance().CommandConfirm(command, acceptInputFrame_))
		{
			return true;
		}
		return false;
	}

	//	アニメーション再生速度の微調整
	void ComboOne2::UpdateAnimationSpeed()
	{
		float currentAnimationSeconds = owner_->GetCurrentAnimationSeconds();

		if (animSpeedChangeInterval_[0].IsJudgeFlag(currentAnimationSeconds))
			owner_->SetAnimationSpeed(1.2f);
		else if (animSpeedChangeInterval_[1].IsJudgeFlag(currentAnimationSeconds))
			owner_->SetAnimationSpeed(1.1f);
		else if (animSpeedChangeInterval_[2].IsJudgeFlag(currentAnimationSeconds))
			owner_->SetAnimationSpeed(1.5f);
		else if (animSpeedChangeInterval_[3].IsJudgeFlag(currentAnimationSeconds))
			owner_->SetAnimationSpeed(1.5f);
	}

	void ComboOne2::Finalize()
	{
		owner_->SetUseRootMotion(false);
		owner_->SetAnimationSpeed(1.0f);

		//	プレイヤーの攻撃判定を無効にする
		owner_->SetAllAttackDetectionActiveFlag(false);
		//	プレイヤーの押し出し判定を有効化
		owner_->SetIsActiveCollisionDetection(true);
	}

	void ComboOne2::DrawDebug()
	{
		if (ImGui::TreeNode("ComboOne2 State"))
		{
			ImGui::DragFloat("StateElapsedTime", &stateElapsedTime_);	//	ステート経過時間
			ImGui::DragFloat("AcceptFrame", &acceptInputFrame_);		//	入力受付フレーム

			//	キャンセル可能時間
			cancellationTime_.DrawDebug();

			ImGui::TreePop();
		}
	}

}

//	コンボ01_3
namespace PlayerState
{
	void ComboOne3::Initialize()
	{
		//	アニメーションセット
		owner_->PlayAnimation(Player::AnimationType::ComboOne3, false, 0.0f);
		owner_->SetAnimationSpeed(1.0f);

		//	ルートモーション
		//owner_->SetUseRootMotion(true);

		//	判定時間セット
		animJudgeTime_[0].SetJudgeTime(0.07f, 0.127f);
		animJudgeTime_[1].SetJudgeTime(0.25f, 0.35f);
		animJudgeTime_[2].SetJudgeTime(0.53f, 0.76f);
		acceptInputFrame_ = 10.0f;
		cancellationTime_.SetJudgeTime(0.7f, 1.6f);
		cancellationTime_.SetName("CancellationTime");

		//	ステート経過時間初期化
		stateElapsedTime_ = 0.0f;

		//	プレイヤーの攻撃判定を無効にする
		owner_->SetAllAttackDetectionActiveFlag(false);
		owner_->SetAttackHit(false);
		//	攻撃中は押し出し判定しない
		owner_->SetIsActiveCollisionDetection(false);
	}

	void ComboOne3::Update(const float& elapsedTime)
	{
		//	一撃目の判定
		float currentAnimationSeconds = owner_->GetCurrentAnimationSeconds();	//	アニメーション再生時間
		if (animJudgeTime_[0].IsJudgeFlag(currentAnimationSeconds))
		{
			owner_->GetAttackDetectionData("LeftPunch").SetIsActive(true);
		}
		//	一撃目のアニメーションが終わったらヒットフラグをオフにする
		if (animJudgeTime_[0].GetMaxTime() < currentAnimationSeconds && currentAnimationSeconds < animJudgeTime_[1].GetMinTime())
			owner_->SetAttackHit(false);

		//	二撃目の判定
		if (animJudgeTime_[1].IsJudgeFlag(currentAnimationSeconds))
		{
			owner_->GetAttackDetectionData("RightPunch").SetIsActive(true);
		}
		//	二撃目のアニメーションが終わったらヒットフラグをオフにする
		if (animJudgeTime_[1].GetMaxTime() < currentAnimationSeconds && currentAnimationSeconds < animJudgeTime_[2].GetMinTime())
			owner_->SetAttackHit(false);

		//	三撃目の判定
		if (animJudgeTime_[2].IsJudgeFlag(currentAnimationSeconds))
		{
			owner_->GetAttackDetectionData("LeftKick").SetIsActive(true);
		}

		//	次のステートへ遷移
		DetermineStateTransition(elapsedTime);

	}

	//	ステートの遷移を判断
	void ComboOne3::DetermineStateTransition(const float& elapsedTime)
	{
		if (JudgeInput(cancellationTime_))	//	入力判定がtrueなら
		{
			//	リズム判定処理(missならreturn)
			if (Rhythm::Instance().GetJudgmentType(Rhythm::Instance().GetCurrentMidiTime()) == Rhythm::JudgmentType::Miss)
				return;

			//	次のステートへ遷移
			owner_->ChangeState(Player::StateType::ComboOne4);
			return;
		}
		if (owner_->IsPlayAnimation() == false)
		{
			owner_->ChangeState(Player::StateType::Idle);
			return;
		}

		//	回避ステートへ遷移
		owner_->ChangeDodgeState();

	}

	bool ComboOne3::JudgeInput(const JudgeTime& cancellationTime)
	{
		//	オートコンボがオンなら入力判定をtrueにする
		if (owner_->IsAutoCombo())
		{
			return true;
		}

		//if (cancellationTime.IsJudgeFlag(stateElapsedTime_) == false)return false;

		if(Input::Instance().GetGamePad().GetButtonDown() & GamePad::BTN_A/*Zキー*/)
		{
			return true;
		}

		return false;

	}

	bool ComboOne3::JudgeInputCommand(const JudgeTime& cancellationTime, const Command& command)
	{
		//	オートコンボがオンなら入力判定をtrueにする
		if (owner_->IsAutoCombo())
		{
			return true;
		}

		if (cancellationTime.IsJudgeFlag(stateElapsedTime_) == false)return false;

		//	判定時間内に指定したボタンが押されていたらisCorrectInput_をtrueにする
		if (Input::Instance().CommandConfirm(command, acceptInputFrame_))
		{
			return true;
		}
	}

	void ComboOne3::Finalize()
	{
		owner_->SetUseRootMotion(false);
		owner_->SetAnimationSpeed(1.0f);

		//	プレイヤーの攻撃判定を無効にする
		owner_->SetAllAttackDetectionActiveFlag(false);
		//	プレイヤーの押し出し判定を有効化
		owner_->SetIsActiveCollisionDetection(true);
	}

	void ComboOne3::DrawDebug()
	{
		if (ImGui::TreeNode("ComboOne3 State"))
		{
			ImGui::DragFloat("StateElapsedTime", &stateElapsedTime_);	//	ステート経過時間
			ImGui::DragFloat("AcceptFrame", &acceptInputFrame_);		//	入力受付フレーム

			//	キャンセル可能時間
			cancellationTime_.DrawDebug();
			ImGui::TreePop();
		}
	}

}

//	コンボ01_4
namespace PlayerState
{
	void ComboOne4::Initialize()
	{
		//	アニメーションセット
		owner_->PlayAnimation(Player::AnimationType::ComboOne4, false, 0.0f);
		owner_->SetAnimationSpeed(1.0f);

		//	ルートモーション
		//owner_->SetUseRootMotion(true);

		//	判定時間セット
		animJudgeTime_.SetJudgeTime(0.4f, 1.185f);
		acceptInputFrame_ = 10.0f;

		//	ステート経過時間初期化
		stateElapsedTime_ = 0.0f;

		//	プレイヤーの攻撃判定を無効にする
		owner_->SetAllAttackDetectionActiveFlag(false);
		owner_->SetAttackHit(false);
		//	攻撃中は押し出し判定しない
		owner_->SetIsActiveCollisionDetection(false);
	}

	void ComboOne4::Update(const float& elapsedTime)
	{
		//	リズム判定をとって判定文字を出すため
		if (Input::Instance().GetGamePad().GetButtonDown() & GamePad::BTN_A/*Zキー*/)
		{
			Rhythm::Instance().GetJudgmentType(Rhythm::Instance().GetCurrentMidiTime());
		}

		float currentAnimationSeconds = owner_->GetCurrentAnimationSeconds();	//	アニメーション再生時間
		if (animJudgeTime_.IsJudgeFlag(currentAnimationSeconds))
		{
			owner_->GetAttackDetectionData("RightPunch").SetIsActive(true);
		}
		
		//	次のステートへ遷移
		DetermineStateTransition(elapsedTime);

	}

	//	ステートの遷移を判断
	void ComboOne4::DetermineStateTransition(const float& elapsedTime)
	{
		if (owner_->IsPlayAnimation() == false)
		{
			owner_->ChangeState(Player::StateType::Idle);

			return;
		}

		//	回避ステートへ遷移
		owner_->ChangeDodgeState();

	}

	bool ComboOne4::JudgeInput(const JudgeTime& cancellationTime)
	{
		//	オートコンボがオンなら入力判定をtrueにする
		if (owner_->IsAutoCombo())
		{
			return true;
		}

		//if (cancellationTime.IsJudgeFlag(stateElapsedTime_) == false)return false;

		if(Input::Instance().GetGamePad().GetButtonDown() & GamePad::BTN_B/*Xキー*/)
		{
			return true;
		}

		return false;
	}

	bool ComboOne4::JudgeInputCommand(const JudgeTime& cancellationTime, const Command& command)
	{
		//	オートコンボがオンなら入力判定をtrueにする
		if (owner_->IsAutoCombo())
		{
			return true;
		}

		if (cancellationTime.IsJudgeFlag(stateElapsedTime_) == false)return false;

		//	判定時間内に指定したボタンが押されていたらisCorrectInput_をtrueにする
		if (Input::Instance().CommandConfirm(command, acceptInputFrame_))
		{
			return true;
		}

		return false;
	}

	void ComboOne4::Finalize()
	{
		owner_->SetUseRootMotion(false);
		owner_->SetAnimationSpeed(1.0f);

		//	プレイヤーの攻撃判定を無効にする
		owner_->SetAllAttackDetectionActiveFlag(false);
		//	プレイヤーの押し出し判定を有効化
		owner_->SetIsActiveCollisionDetection(true);
	}

	void ComboOne4::DrawDebug()
	{
		if (ImGui::TreeNode("ComboOne4 State"))
		{
			ImGui::DragFloat("StateElapsedTime", &stateElapsedTime_);	//	ステート経過時間
			ImGui::DragFloat("AcceptFrame", &acceptInputFrame_);		//	入力受付フレーム

			float cancellationTimeMin = cancellationTime_.GetMinTime();	//	キャンセル可能時間
			float cancellationTimeMax = cancellationTime_.GetMaxTime();
			ImGui::DragFloat("CancelTimeMin", &cancellationTimeMin);
			ImGui::DragFloat("CancelTimeMin", &cancellationTimeMax);
			cancellationTime_.SetJudgeTime(cancellationTimeMin, cancellationTimeMax);

			ImGui::TreePop();
		}
	}
}

//	回避ステート
namespace PlayerState
{
	void DodgeState::Initialize()
	{
		//	----- アニメーション再生設定 -----
		owner_->PlayAnimation(Player::AnimationType::DodgeBack, false, 0.0f, 1.0f, startFrame_, endFrame_);
		owner_->SetAnimationSpeed(1.0f);
		animSpeedChangeInterval_[0].SetJudgeTime(0.0f, 0.59f);
		animSpeedChangeInterval_[0].SetName("AnimSpeedInterval0");
		animSpeedChangeInterval_[1].SetJudgeTime(0.60f, 1.333f);
		animSpeedChangeInterval_[1].SetName("AnimSpeedInterval1");

		//	----- ルートモーション -----
		owner_->SetUseRootMotion(true);
	}

	void DodgeState::Update(const float& elapsedTime)
	{
		//	----- ルートモーション設定 -----
		if (owner_->IsBlendAnimation() == false)
		{
			owner_->SetRootMotionSpeed(-rootMotionSpeed_);
		}

		//	回避移動
		//MoveForward(elapsedTime);
		
		//	次のステートへ遷移
		DetermineStateTransition(elapsedTime);

	}

	//	アニメーション再生速度を調整
	void DodgeState::UpdateAnimationSpeed()
	{
		//	現在のアニメーション再生時間
		float currentAnimationSeconds = owner_->GetCurrentAnimationSeconds();

		//	アニメーション速度更新
		for (int i = 0; i < AnimSpeedSectionCount_; ++i)
		{
			if (animSpeedChangeInterval_[i].IsJudgeFlag(currentAnimationSeconds))
			{
				owner_->SetAnimationSpeed(animationSpeed_[i]);
				break;
			}
		}
	}

	//	回避移動
	void DodgeState::MoveForward(const float& elapsedTime)
	{
		//	プレイヤーの前方向
		DirectX::XMFLOAT3 moveVec = owner_->GetMoveVec();
		//DirectX::XMFLOAT3 playerForward = owner_->GetTransform()->CalcForward();
		moveVec.y = 0.0f;

		owner_->AddForce(moveVec, moveLength_, decelerationForce_);

	}

	//	ステートの遷移を判断	
	void DodgeState::DetermineStateTransition(const float& elapsedTime)
	{
		//	アニメーション再生中なら遷移しない
		if (owner_->IsPlayAnimation())return;
		
		//	移動入力があれば移動ステートへ遷移
		if (owner_->InputMove(elapsedTime))
		{
			owner_->ChangeState(Player::StateType::Move);
			return;
		}

		//	待機ステートへ遷移
		owner_->ChangeState(Player::StateType::Idle);
		return;
	}

	void DodgeState::Finalize()
	{
		owner_->SetAnimationSpeed(1.0f);
		owner_->SetUseRootMotion(false);
	}

	void DodgeState::DrawDebug()
	{
		if (ImGui::TreeNode("DodgeState"))
		{
			ImGui::DragFloat("StateElapsedTime", &stateElapsedTime_, 0.1f);

			if (ImGui::TreeNode("Animation"))
			{
				//	アニメーション再生フレーム
				ImGui::DragFloat("StartFrame", &startFrame_,0.01f);
				ImGui::DragFloat("EndFrame", &endFrame_, 0.01f);

				//	アニメーション速度を変化させる区間	
				for (int i = 0; i < AnimSpeedSectionCount_; ++i)
				{
					animSpeedChangeInterval_[i].DrawDebug();
				}

				//	設定した区間のアニメーション速度
				ImGui::DragFloat("AnimSpeed0", &animationSpeed_[0]);
				ImGui::DragFloat("AnimSpeed1", &animationSpeed_[1]);
				ImGui::DragFloat("AnimSpeed2", &animationSpeed_[2]);

				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Move"))
			{
				ImGui::DragFloat("MoveLength", &moveLength_);
				ImGui::DragFloat("DecelerationForce", &decelerationForce_);
				ImGui::DragFloat("RootMotionSpeed", &rootMotionSpeed_);

				ImGui::TreePop();
			}
			ImGui::TreePop();
		}
	}
}

//	ダメージステート
namespace PlayerState
{
	void DamageState::Initialize()
	{
		//	コントローラー振動設定
		SetGamePadVibration();

		//	アニメーションの速度を変える区間を設定
		animSpeedChangeInterval_[0].SetJudgeTime(0.0f, 0.54f);		//	地面につく
		animSpeedChangeInterval_[1].SetJudgeTime(0.55f, 1.1f);		//	動作終わり

		//	吹っ飛びアニメーション再生
		owner_->PlayAnimation(Player::AnimationType::HitDeath, false, 0.1f, 1.0f, startFrame_, endFrame_);
		owner_->SetAnimationSpeed(1.5f);

		//  敵からプレイヤーの方向へ吹っ飛ばす
		DirectX::XMFLOAT3 direction = Normalize(owner_->GetTransform()->GetPosition() - owner_->GetEnemyPos());
		direction.y = 0.0f;
		owner_->AddForce(direction, blowPower_, decelerationForce_);

		//	吹っ飛び方向から回転角度を求める
		float blowbackRotationY = atan2(direction.x, direction.z) + DirectX::XM_PI;

		//	吹っ飛び方向に応じた回転処理
		owner_->GetTransform()->SetRotationY(blowbackRotationY); // 吹っ飛ぶ方向に回転
	}

	void DamageState::Update(const float& elapsedTime)
	{
		//	アニメーション速度調整
		UpdateAnimationSpeed();

		//	次のステートへ遷移
		DetermineStateTransition(elapsedTime);

	}

	//	ステートの遷移を判断
	void DamageState::DetermineStateTransition(const float& elapsedTime)
	{
		//	アニメーション再生が終わったらステート終了
		if (owner_->IsPlayAnimation() == false)
		{
			owner_->ChangeState(Player::StateType::GetUp);
		}
	}

	void DamageState::UpdateAnimationSpeed()
	{
		//	現在のアニメーション再生時間
		float currentAnimationSeconds = owner_->GetCurrentAnimationSeconds();

		//	アニメーション速度更新
		for (int i = 0; i < AnimSpeedSectionCount_; ++i)
		{
			if (animSpeedChangeInterval_[i].IsJudgeFlag(currentAnimationSeconds))
			{
				owner_->SetAnimationSpeed(animationSpeed_[i]);
				break;
			}
		}
	}

	//	コントローラー振動
	void DamageState::SetGamePadVibration()
	{
		GamePad& gamePad = Input::Instance().GetGamePad();
		gamePad.SetVibration(leftVibrationPower_, rightVibrationPower_, vibrationTime_);

	}

	void DamageState::Finalize()
	{
		owner_->SetAnimationSpeed(1.0f);
		owner_->GetTransform()->SetRotationY(0.0f);
	}

	void DamageState::DrawDebug()
	{
		if (ImGui::TreeNode("DamageState"))
		{
			ImGui::DragFloat("StateElapsedTime", &stateElapsedTime_, 0.1f);

			if (ImGui::TreeNode("PlayDuration"))
			{
				ImGui::DragFloat("StartFrame",	&startFrame_, 0.01f);
				ImGui::DragFloat("EndFrame",	&endFrame_, 0.01f);
				ImGui::TreePop();
			}

			if(ImGui::TreeNode("AnimationSpeed"))
			{
				ImGui::DragFloat("AnimSpeedSection0", &animationSpeed_[0]);
				ImGui::DragFloat("AnimSpeedSection1", &animationSpeed_[1]);
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("AddForce"))
			{
				ImGui::DragFloat("BlowPower",			&blowPower_);
				ImGui::DragFloat("DecelerationForce",	&decelerationForce_);

				ImGui::TreePop();
			}

			if (ImGui::TreeNode("GamePadVibration"))
			{
				ImGui::DragFloat("LeftPower", &leftVibrationPower_, 0.01f);
				ImGui::DragFloat("RightPower", &rightVibrationPower_, 0.01f);
				ImGui::DragFloat("Time", &vibrationTime_, 0.01f);

				ImGui::TreePop();
			}

			ImGui::TreePop();
		}
	}
}

//	起き上がりステート
namespace PlayerState
{
	void GetUpState::Initialize()
	{
		//	起き上がりアニメーション再生
		owner_->PlayAnimation(Player::AnimationType::GetUp, false, 0.1f, animationSpeed_, startFrame_, endFrame_);

		//  敵からプレイヤーの方向へ吹っ飛ばす
		DirectX::XMFLOAT3 direction = Normalize(owner_->GetTransform()->GetPosition() - owner_->GetEnemyPos());
		direction.y = 0.0f;
	
		//	吹っ飛び方向から回転角度を求める
		float blowbackRotationY = atan2(direction.x, direction.z) + DirectX::XM_PI;

		//	吹っ飛び方向に応じた回転処理
		owner_->GetTransform()->SetRotationY(blowbackRotationY); // 吹っ飛ぶ方向に回転

	}

	void GetUpState::Update(const float& elapsedTime)
	{
		//	ステート遷移
		DetermineStateTransition();

	}

	//	ステートの遷移を判断
	void GetUpState::DetermineStateTransition()
	{
		//	起き上がりアニメーション再生が終わったらステート終了
		if (owner_->IsPlayAnimation() == false)
		{
			owner_->ChangeState(Player::StateType::Idle);
		}
	}

	void GetUpState::Finalize()
	{

	}

	void GetUpState::DrawDebug()
	{
		if (ImGui::TreeNode("GetUpState"))
		{
			ImGui::DragFloat("StateElapsedTime", &stateElapsedTime_, 0.1f);

			ImGui::DragFloat("StartFrame", &startFrame_, 0.01f);
			ImGui::DragFloat("EndFrame", &endFrame_, 0.01f);
			ImGui::DragFloat("AnimSpeed", &animationSpeed_, 0.01f);

			ImGui::TreePop();
		}
	}

}

//	怯みステート
namespace PlayerState
{
	void FlinchState::Initialize()
	{

	}

	void FlinchState::Update(const float& elapsedTime)
	{
		//	次のステートへ遷移
		DetermineStateTransition(elapsedTime);

	}

	//	ステートの遷移を判断
	void FlinchState::DetermineStateTransition(const float& elapsedTime)
	{
		owner_->ChangeState(Player::StateType::Idle);
	}

	void FlinchState::Finalize()
	{

	}

	void FlinchState::DrawDebug()
	{
		if (ImGui::TreeNode("FlinchState"))
		{
			ImGui::DragFloat("StateElapsedTime", &stateElapsedTime_, 0.1f);

			ImGui::TreePop();
		}
	}
}

//	死亡ステート
namespace PlayerState
{
	void DeathState::Initialize()
	{
		owner_->PlayAnimation(Player::AnimationType::HitDeath, false, 0.1f, 1.0f, startFrame_, endFrame_);
		owner_->SetAnimationSpeed(1.5f);
	}

	void DeathState::Update(const float& elapsedTime)
	{
		//	次のステートへ遷移
		DetermineStateTransition(elapsedTime);

	}

	//	ステートの遷移を判断
	void DeathState::DetermineStateTransition(const float& elapsedTime)
	{
		//	アニメーション再生が終わっても遷移しない
	}

	void DeathState::Finalize()
	{

	}

	void DeathState::DrawDebug()
	{
		if (ImGui::TreeNode("DeathState"))
		{
			ImGui::DragFloat("StateElapsedTime", &stateElapsedTime_, 0.1f);

			ImGui::DragFloat("StartFrame", &startFrame_, 0.01f);
			ImGui::DragFloat("EndFrame", &endFrame_, 0.01f);

			ImGui::TreePop();
		}
	}

}

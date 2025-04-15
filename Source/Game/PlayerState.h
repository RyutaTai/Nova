#pragma once

#include "Player.h"
#include "../Nova/AI/State.h"
#include "../Nova/Others/JudgeTime.h"

//	待機ステート
namespace PlayerState
{
	class IdleState :public State<Player>
	{
	public:
		IdleState(Player* owner) : State(owner) {}
		~IdleState() {}

		void Initialize()override;
		void Update(const float& elapsedTime)override;
		void Finalize()override;
		void DrawDebug()override;

		//	----- ステートの遷移を判断 -----
		void DetermineStateTransition(const float& elapssedTime);

	private:
		//	----- アニメーション再生 -----
		float blendAnimTime_ = 0.2f;	//	アニメーションブレンド時間

	};
}

//	移動ステート
namespace PlayerState
{
	class MoveState :public State<Player>
	{
	public:
		MoveState(Player* owner) : State(owner) {}
		~MoveState() {}

		void Initialize()override;
		void Update(const float& elapsedTime)override;
		void Finalize()override;
		void DrawDebug()override;

		//	----- ステートの遷移を判断 -----
		void DetermineStateTransition(const float& elapssedTime);

	private:
		//	----- 足音再生 -----
		void PlayFootstepsSE(const float& elapsedTime);

	private:
		//	----- 移動速度 -----
		float moveSpeed_ = 9.0f;

		//	----- アニメーション再生 -----
		float blendAnimTime_ = 0.25f;	//	アニメーションブレンド時間

		//	----- 足音SE -----
		float playFootstepsInterval_	= 0.278f;		//	足音SE再生間隔
		float footStepsTimer_			= 0.0f;			//	足音SE再生間隔用タイマー

	};
}

//	コンボ01_1(右パンチ)
namespace PlayerState
{
	class ComboOne1 :public State<Player>
	{
	public:
		ComboOne1(Player* owner) :State(owner) {}
		~ComboOne1(){}

		void Initialize()override;
		void Update(const float& elapsedTime)override;
		void Finalize()override;
		void DrawDebug()override;

	private:
		//	----- 入力判定 -----
		bool JudgeInput(const JudgeTime& inputJudgeTime);	//	正しい入力が取れていたらtrue
		bool JudgeInputCommand(const JudgeTime& inputJudgeTime, const Command& command);
		
		//	----- アニメーション再生速度を調整 -----
		void UpdateAnimationSpeed();	//	アニメーション箇所で速度を変化

		//	----- ステートの遷移を判断 -----
		void DetermineStateTransition(const float& elapsedTime);

	private:
		JudgeTime	animJudgeTime_	= {};				//	判定を取るアニメーション区間
		JudgeTime	animSpeedChangeInterval_[3] = {};	//	再生速度を変更するアニメーション区間
		JudgeTime	cancellationTime_ = {};				//	キャンセル可能時間
		float		acceptInputFrame_ = 0.0f;			//	入力時間を受け付ける時間(CommandConfirm関数でさかのぼるフレーム数)

	};
}

//	コンボ01_2
namespace PlayerState
{
	class ComboOne2 :public State<Player>
	{
	public:
		ComboOne2(Player* owner) :State(owner) {}
		~ComboOne2() {}

		void Initialize()override;
		void Update(const float& elapsedTime)override;
		void Finalize()override;
		void DrawDebug()override;

	private:
		//	----- 入力判定 -----
		bool JudgeInput(const JudgeTime& cancellationTime);
		bool JudgeInputCommand(const JudgeTime& inputJudgeTime, const Command& command);
		
		//	----- アニメーション再生速度を調整 -----
		void UpdateAnimationSpeed();

		//	----- ステートの遷移を判断 -----
		void DetermineStateTransition(const float& elapedTime);

	private:
		static const int	AnimJudgeCount_ = 2;					//	アニメーション判定区間の数
		JudgeTime			animJudgeTime_[AnimJudgeCount_] = {};	//	判定を取るアニメーション区間
		
		static const int	AnimSpeedSectionCount_ = 4;								//	アニメーション速度変化区間の数
		JudgeTime			animSpeedChangeInterval_[AnimSpeedSectionCount_] = {};	//	再生速度を変更するアニメーション区間
		JudgeTime			cancellationTime_ = {};									//	キャンセル可能時間
		float				acceptInputFrame_ = 0.0f;								//	入力時間を受け付ける範囲
		bool				isHit_ = false;											//	このコンボの最後の攻撃があたったらtrue

	};
}

//	コンボ01_3(右アッパー)
namespace PlayerState
{
	class ComboOne3 :public State<Player>
	{
	public:
		ComboOne3(Player* owner) :State(owner) {}
		~ComboOne3() {}

		void Initialize()override;
		void Update(const float& elapsedTime)override;
		void Finalize()override;
		void DrawDebug()override;

	private:
		//	----- 入力判定 -----
		bool JudgeInput(const JudgeTime& cancellationTime);	//	正しい入力が取れていたらtrue
		bool JudgeInputCommand(const JudgeTime& inputJudgeTime, const Command& command);

		//	----- ステートの遷移を判断 -----
		void DetermineStateTransition(const float& elapedTime);

	private:
		static const int	AnimJudgeCount_ = 3;					//	アニメーション判定区間の数
		JudgeTime			animJudgeTime_[AnimJudgeCount_] = {};	//	判定を取るアニメーション区間
		JudgeTime			cancellationTime_ = {};					//	キャンセル可能時間
		float				acceptInputFrame_ = {};					//	入力時間を受け付ける範囲

	};
}

//	コンボ01_4
namespace PlayerState
{
	class ComboOne4 :public State<Player>
	{
	public:
		ComboOne4(Player* owner) :State(owner) {}
		~ComboOne4() {}

		void Initialize()override;
		void Update(const float& elapsedTime)override;
		void Finalize()override;
		void DrawDebug()override;

	private:
		//	----- 入力判定 -----
		bool JudgeInput(const JudgeTime& cancellationTime);	//	正しい入力が取れていたらtrue
		bool JudgeInputCommand(const JudgeTime& inputJudgeTime, const Command& command);

		//	----- ステートの遷移を判断 -----
		void DetermineStateTransition(const float& elapsedTime);

	private:
		JudgeTime	animJudgeTime_	= {};		//	判定を取るアニメーション区間
		JudgeTime	cancellationTime_ = {};		//	キャンセル可能時間
		float		acceptInputFrame_ = {};		//	入力時間を受け付ける範囲

	};
}

//	回避ステート
namespace PlayerState
{
	class DodgeState :public State<Player>
	{
	public:
		DodgeState(Player* owner) : State(owner) {}
		~DodgeState() {}

		void Initialize()override;
		void Update(const float& elapsedTime)override;
		void Finalize()override;
		void DrawDebug()override;

	private:
		//	----- ステートの遷移を判断 -----
		void DetermineStateTransition(const float& elapsedTime);

		//	----- アニメーション再生速度を調整 -----
		void UpdateAnimationSpeed();

		//	----- 移動 -----
		void MoveForward(const float& elapsedTime);
		
	private:
		//	----- 移動 -----
		float moveLength_ = 2.0f;			//	移動量
		float decelerationForce_ = 4.0f;	//	

		//	----- ルートモーション -----
		float rootMotionSpeed_ = 1.0f;

		//	----- アニメーション -----
		static const int	AnimSpeedSectionCount_ = 2;									//	アニメーション速度変化区間の数
		JudgeTime			animSpeedChangeInterval_[AnimSpeedSectionCount_] = {};		//	再生速度を変更するアニメーション区間
		float				animationSpeed_[AnimSpeedSectionCount_] = { 1.0f,1.5f };	//	各区間のアニメーション速度

		float				startFrame_ = 0.19f;	//	アニメーションの再生開始位置
		float				endFrame_ = 0.96f;		//	回避アニメーションの長さで初期化

	};
}

//	ダメージステート
namespace PlayerState
{
	class DamageState :public State<Player>
	{
	public:
		DamageState(Player* owner) :State(owner) {}
		~DamageState() {}

		void Initialize()override;
		void Update(const float& elapsedTime)override;
		void Finalize()override;
		void DrawDebug()override;

	private:
		//	----- アニメーション再生速度を調整 -----
		void UpdateAnimationSpeed();

		//	----- ステートの遷移を判断 -----
		void DetermineStateTransition(const float& elapsedTime);

		//	----- コントローラー振動 -----
		void SetGamePadVibration();

	private:
		//	----- アニメーション再生 -----
		float startFrame_ = 0.35f;
		float endFrame_ = 1.0f;
		
		//	----- アニメーション速度 -----
		static const int	AnimSpeedSectionCount_								= 2;					//	アニメーション速度変化区間の数
		JudgeTime			animSpeedChangeInterval_[AnimSpeedSectionCount_]	= {};					//	再生速度を変更するアニメーション区間
		float				animationSpeed_[AnimSpeedSectionCount_]				= { 1.2f,2.8f};	//	各区間のアニメーション速度

		//	----- 吹っ飛ばし -----
		float blowPower_ = 20.0f;			//	吹っ飛ばし力
		float decelerationForce_ = 30.0f;	//	

		//	----- コントローラー振動 -----
		float leftVibrationPower_ = 0.0f;
		float rightVibrationPower_ = 0.0f;
		float vibrationTime_ = 0.0f;

	};
}

//	起き上がりステート
namespace PlayerState
{
	class GetUpState :public State<Player>
	{
	public:
		GetUpState(Player* owner) :State(owner) {}
		~GetUpState() {}

		void Initialize()override;
		void Update(const float& elapsedTime)override;
		void Finalize()override;
		void DrawDebug()override;

	private:
		// ----- ステート遷移を判断 -----
		void DetermineStateTransition();

	private:
		//	----- アニメーション再生 -----
		float startFrame_ = 0.13f;
		float endFrame_ = 1.0f;

		float animationSpeed_ = 1.55f;

	};
}

//	怯みステート
namespace PlayerState
{
	class FlinchState :public State<Player>
	{
	public:
		FlinchState(Player* owner) :State(owner) {}
		~FlinchState() {}

		void Initialize()override;
		void Update(const float& elapsedTime)override;
		void Finalize()override;
		void DrawDebug()override;

	private:
		//	----- ステートの遷移を判断 -----
		void DetermineStateTransition(const float& elapsedTime);

	};
}

//	死亡ステート
namespace PlayerState
{
	class DeathState :public State<Player>
	{
	public:
		DeathState(Player* owner) :State(owner) {}
		~DeathState(){}

		void Initialize()override;
		void Update(const float& elapsedTime)override;
		void Finalize()override;
		void DrawDebug()override;

	private:
		//	----- ステートの遷移を判断 -----
		void DetermineStateTransition(const float& elapsedTime);

	private:
		//	----- アニメーション -----
		float startFrame_ = 0.0f;
		float endFrame_ = 2.2f;

	};
}
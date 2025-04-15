#pragma once

#include "../Nova/AI/State.h"
#include "Drone.h"

//	待機ステート
namespace DroneState
{
	class IdleState : public State<Drone>
	{
	public:
		IdleState(Drone* owner) : State(owner){}
		~IdleState(){}

		void Initialize()override;
		void Update(const float& elapsedTime)override;
		void Finalize()override;
		void DrawDebug()override;

	private:
		float IdleTime_ = 1.0f;			//	待機に入って静止する時間
		//const float IdleTime_ = 1.0f;	//	待機に入って静止する時間

	};
}

//	サーチステート
namespace DroneState
{
	class SearchState :public State<Drone>
	{
	public:
		SearchState(Drone* owner) : State(owner) {}
		~SearchState() {}

		void Initialize()override;
		void Update(const float& elapsedTime)override;
		void Finalize()override;
		void DrawDebug()override;

	};
}

//	移動ステート
namespace DroneState
{
	class MoveState :public State<Drone>
	{
	public:
		MoveState(Drone* owner) : State(owner) {}
		~MoveState() {}

		void Initialize()override;
		void Update(const float& elapsedTime)override;
		void Finalize()override;
		void DrawDebug()override;

	};
}

//	追跡ステート
namespace DroneState
{
	class PursuitState :public State<Drone>
	{
	public:
		PursuitState(Drone* owner) :State(owner) {}
		~PursuitState() {}

		void Initialize()override;
		void Update(const float& elapsedTime)override;
		void Finalize()override;
		void DrawDebug()override;

	};
}

//	攻撃ステート
namespace DroneState
{
	class AttackState :public State<Drone>
	{
	public:
		AttackState(Drone* owner) : State(owner) {}
		~AttackState() {}

		void Initialize()override;
		void Update(const float& elapsedTime)override;
		void Finalize()override;
		void DrawDebug()override;

	private:
		float initLaunchTimer_ = 1.0f;

	};
}

//	回避ステート
namespace DroneState
{
	class AvoidanceState :public State<Drone>
	{
	public:
		AvoidanceState(Drone* owner) : State(owner) {}
		~AvoidanceState() {}

		void Initialize()override;
		void Update(const float& elapsedTime)override;
		void Finalize()override;
		void DrawDebug()override;

	};
}

//	ダメージステート　
namespace DroneState
{
	class DamageState :public State<Drone>
	{
	public:
		DamageState(Drone* owner) :State(owner) {}
		~DamageState() {}

		void Initialize()override;
		void Update(const float& elapsedTime)override;
		void Finalize()override;
		void DrawDebug()override;

	};

}

//	死亡ステート
namespace DroneState
{
	class DeathState :public State<Drone>
	{
	public:
		DeathState(Drone* owner) :State(owner) {}
		~DeathState() {}

		void Initialize()override;
		void Update(const float& elapsedTime)override;
		void Finalize()override;
		void DrawDebug()override;

	private:
		float destroyTimer_		= 0.0f;	//	破棄するまでのタイマー
		float destroyDuration_	= 2.0f;	//	破棄するまでの時間

	};

}
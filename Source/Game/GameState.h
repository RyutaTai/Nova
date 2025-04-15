#pragma once

#include "../Nova/AI/State.h"
#include "../Nova/Scenes/SceneGame.h"

//	ウェーブ1（敵1体）
namespace GameState
{
	class Wave1State : public State<SceneGame>
	{
	public:
		Wave1State(SceneGame* owner) : State(owner) {}
		~Wave1State() {}

		void Initialize()override;
		void Update(const float& elapsedTime)override;
		void Finalize()override;
		void DrawDebug()override;

	};
}

//	ウェーブ2（敵2体）
namespace GameState
{
	class Wave2State : public State<SceneGame>
	{
	public:
		Wave2State(SceneGame* owner) : State(owner) {}
		~Wave2State() {}

		void Initialize()override;
		void Update(const float& elapsedTime)override;
		void Finalize()override;
		void DrawDebug()override;

	};
}

//	ウェーブ3（敵5体）
namespace GameState
{
	class Wave3State : public State<SceneGame>
	{
	public:
		Wave3State(SceneGame* owner) : State(owner) {}
		~Wave3State() {}

		void Initialize()override;
		void Update(const float& elapsedTime)override;
		void Finalize()override;
		void DrawDebug()override;

	};
}

//	ボス
namespace GameState
{
	class BossState : public State<SceneGame>
	{
	public:
		BossState(SceneGame* owner) : State(owner) {}
		~BossState() {}

		void Initialize()override;
		void Update(const float& elapsedTime)override;
		void Finalize()override;
		void DrawDebug()override;

	};
}

//	ゲームクリア
namespace GameState
{
	class GameClearState : public State<SceneGame>
	{
	public:
		GameClearState(SceneGame* owner) : State(owner) {}
		~GameClearState() {}

		void Initialize()override;
		void Update(const float& elapsedTime)override;
		void Finalize()override;
		void DrawDebug()override;

	private:
		float changeTitleTimer_ = 4.0f;

	};
}

//	ゲームオーバー
namespace GameState
{
	class GameOverState : public State<SceneGame>
	{
	public:
		GameOverState(SceneGame* owner) : State(owner) {}
		~GameOverState() {}

		void Initialize()override;
		void Update(const float& elapsedTime)override;
		void Finalize()override;
		void DrawDebug()override;

	private:
		float changeTitleTimer_ = 4.0f;

	};
}

//	コンティニュー
namespace GameState
{
	class ContinueState : public State <SceneGame>
	{
	public:
		ContinueState(SceneGame* owner) :State(owner) {}
		~ContinueState() {}

		void Initialize()override;
		void Update(const float& elapsedTime)override;
		void Finalize()override;
		void DrawDebug()override;

	};
}

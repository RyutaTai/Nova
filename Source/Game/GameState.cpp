#include "GameState.h"

#include "../Nova/Scenes/SceneGame.h"
#include "EnemyManager.h"

#define USE_CONTINUE 0

//	ウェーブ1（敵1体）
namespace GameState
{
	void Wave1State::Initialize()
	{
		EnemyManager::Instance().DroneSpawn(1);
		owner_->LoadWaveSprite(L"./Resources/Image/Wave1.png");
		owner_->SetWaveStartTimer(2.0f);

		//	オーディオ再生
		AudioManager::Instance().GetAudioResource("GameBGM")->Play(true);
	}

	void Wave1State::Update(const float& elapsedTime)
	{
		//	ウェーブ開始タイマー更新
		float waveStartTimer = owner_->GetWaveStartTimer();
		waveStartTimer -= elapsedTime;
		if (waveStartTimer <= 0.0f)
		{
			waveStartTimer = 0.0f;
		}
		owner_->SetWaveStartTimer(waveStartTimer);

		//	ステート遷移
		if (EnemyManager::Instance().GetEnemyCount() <= 0)
		{
			//owner_->ChangeState(SceneGame::SceneGameState::Wave2);
		}

		//	ステート遷移
		if (EnemyManager::Instance().GetEnemyCount() <= 0)
		{
			owner_->ChangeState(SceneGame::SceneGameState::Clear);
		}

	}

	void Wave1State::Finalize()
	{

	}

	void Wave1State::DrawDebug()
	{
		if (ImGui::TreeNode("Wave1State"))
		{
			ImGui::DragFloat("StateElapsedTime", &stateElapsedTime_, 0.1f);

			ImGui::TreePop();
		}
	}

}

//	ウェーブ2（敵2体）
namespace GameState
{
	void Wave2State::Initialize()
	{
		EnemyManager::Instance().DroneSpawn(3);
		owner_->LoadWaveSprite(L"./Resources/Image/Wave2.png");
		owner_->SetWaveStartTimer(2.0f);
	}

	void Wave2State::Update(const float& elapsedTime)
	{
		//	ウェーブ開始タイマー更新
		float waveStartTimer = owner_->GetWaveStartTimer();
		waveStartTimer -= elapsedTime;
		if (waveStartTimer <= 0.0f)
		{
			waveStartTimer = 0.0f;
		}
		owner_->SetWaveStartTimer(waveStartTimer);

		//	ステート遷移
		if (EnemyManager::Instance().GetEnemyCount() <= 0)
		{
			owner_->ChangeState(SceneGame::SceneGameState::Wave3);
		}

	}

	void Wave2State::Finalize()
	{

	}

	void Wave2State::DrawDebug()
	{
		if (ImGui::TreeNode("Wave2State"))
		{
			ImGui::DragFloat("StateElapsedTime", &stateElapsedTime_, 0.1f);

			ImGui::TreePop();
		}
	}

}

//	ウェーブ3（敵5体）
namespace GameState
{
	void Wave3State::Initialize()
	{
		EnemyManager::Instance().DroneSpawn(5);
		owner_->LoadWaveSprite(L"./Resources/Image/Wave3.png");
		owner_->SetWaveStartTimer(2.0f);
	}

	void Wave3State::Update(const float& elapsedTime)
	{
		//	ウェーブ開始タイマー更新
		float waveStartTimer = owner_->GetWaveStartTimer();
		waveStartTimer -= elapsedTime;
		if (waveStartTimer <= 0.0f)
		{
			waveStartTimer = 0.0f;
		}
		owner_->SetWaveStartTimer(waveStartTimer);

		//	ステート遷移
		if (EnemyManager::Instance().GetEnemyCount() <= 0)
		{
			owner_->ChangeState(SceneGame::SceneGameState::Clear);
		}

	}

	void Wave3State::Finalize()
	{

	}

	void Wave3State::DrawDebug()
	{
		if (ImGui::TreeNode("Wave3State"))
		{
			ImGui::DragFloat("StateElapsedTime", &stateElapsedTime_, 0.1f);

			ImGui::TreePop();
		}
	}

}

//	ゲームクリア
namespace GameState
{
	void GameClearState::Initialize()
	{
		owner_->SetGameClear(true);
		owner_->SetIsResult(true);
	}

	void GameClearState::Update(const float& elapsedTime)
	{
		changeTitleTimer_ -= elapsedTime;
		if (changeTitleTimer_ <= 0.0f)
		{
#if USE_CONTINUE
			owner_->ChangeState(SceneGame::SceneGameState::Continue);
#else
			owner_->ChangeToTitle(true);
#endif
			changeTitleTimer_ = 3.0f;
		}
	}

	void GameClearState::Finalize()
	{
		owner_->SetGameClear(false);
		owner_->SetIsResult(false);
	}

	void GameClearState::DrawDebug()
	{
		if (ImGui::TreeNode("GameClearState"))
		{
			ImGui::DragFloat("StateElapsedTime", &stateElapsedTime_, 0.1f);

			ImGui::TreePop();
		}
	}

}

//	ゲームオーバー
namespace GameState
{
	void GameOverState::Initialize()
	{
		owner_->SetGameOver(true);
		owner_->SetIsResult(true);
	}

	void GameOverState::Update(const float& elapsedTime)
	{
		changeTitleTimer_ -= elapsedTime;
		if (changeTitleTimer_ <= 0.0f)
		{
#if USE_CONTINUE
			owner_->ChangeState(SceneGame::SceneGameState::Continue);
#else
			owner_->ChangeToTitle(true);
#endif
			changeTitleTimer_ = 3.0f;
		}
	}

	void GameOverState::Finalize()
	{
		owner_->SetGameOver(false);
		owner_->SetIsResult(false);
	}

	void GameOverState::DrawDebug()
	{
		if (ImGui::TreeNode("GameOverState"))
		{
			ImGui::DragFloat("StateElapsedTime", &stateElapsedTime_, 0.1f);

			ImGui::TreePop();
		}
	}

}

//	コンティニュー
namespace GameState
{
	void ContinueState::Initialize()
	{
		owner_->Finalize();
		owner_->Initialize();	//	Initialize()内でChangeState()で初期ステートを設定しているため、このステートでやらない
	}

	void ContinueState::Update(const float& elapsedTime)
	{

	}

	void ContinueState::Finalize()
	{

	}

	void ContinueState::DrawDebug()
	{
		if (ImGui::TreeNode("ContinueState"))
		{
			ImGui::DragFloat("StateElapsedTime", &stateElapsedTime_, 0.1f);

			ImGui::TreePop();
		}
	}

}
#include "GameState.h"

#include "../../Game/Scenes/SceneGame.h"
#include "../Character/Enemy/EnemyManager.h"

//	ウェーブ1（敵1体）
namespace GameState
{
	void Wave1State::Initialize()
	{
		EnemyManager::Instance().DroneSpawn(1);
		owner_->LoadWaveSprite(L"./Resources/Image/Wave1.png");
		owner_->SetWaveStartTimer(2.0f);

		//	オーディオ再生
		AudioManager::Instance().PlayAudioByName("GameBGM", true);
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

		//	BGMの音量下げる
		AudioManager::Instance().GetAudioResource("GameBGM")->SetVolume(0.1f, false);

		//	クリアSE再生
		AudioSource* clearSE = AudioManager::Instance().LoadAudioSource("./Resources/Audio/SE/Clear.wav", Audio::AudioType::SENormal, "GameScene");
		volume_ = 0.25f;
		clearSE->SetVolume(volume_, false);
		clearSE->SetAudioName("ClearSE");
		AudioManager::Instance().Register(clearSE);
		clearSE->Play(false);
	}

	void GameClearState::Update(const float& elapsedTime)
	{
		//	bgmの音量をだんだん下げる
		volume_ -= 0.1f * elapsedTime;
		AudioManager::Instance().GetAudioResource("GameBGM")->SetVolume(volume_, false);

		//	一定時間経過後にタイトルへ遷移
		changeTitleTimer_ -= elapsedTime;
		if (changeTitleTimer_ <= 0.0f)
		{
			owner_->ChangeToTitle(true);
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

		//	クリアSE再生
		AudioSource* gameOverSE = AudioManager::Instance().LoadAudioSource("./Resources/Audio/SE/GameOver.wav", Audio::AudioType::SENormal, "GameScene");
		volume_ = 0.25f;
		gameOverSE->SetVolume(volume_, false);
		gameOverSE->SetAudioName("GameOverSE");
		AudioManager::Instance().Register(gameOverSE);
		gameOverSE->Play(false);
	}

	void GameOverState::Update(const float& elapsedTime)
	{
		//	bgmの音量をだんだん下げる
		volume_ -= 0.1f * elapsedTime;
		AudioManager::Instance().GetAudioResource("GameBGM")->SetVolume(volume_, false);

		//	一定時間経過後にタイトルへ遷移
		changeTitleTimer_ -= elapsedTime;
		if (changeTitleTimer_ <= 0.0f)
		{
			owner_->ChangeToTitle(true);
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

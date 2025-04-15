#include "SceneTitle.h"

#include "../Graphics/Graphics.h"
#include "../Resources/Texture.h"
#include "../Scenes/SceneManager.h"
#include "../Scenes/SceneLoading.h"
#include "../Scenes/SceneGame.h"
#include "../../Game/UI/UIManager.h"
#include "../../Game/TitleState.h"
#include "../Audio/AudioManager.h"

//	初期化
void SceneTitle::Initialize()
{
	//	オーディオ初期化
	AudioSource* titleBGM = AudioManager::Instance().LoadAudioSource("./Resources/Audio/BGM/Title.wav", Audio::AudioType::BGMNormal, "TitleScene");
	titleBGM->SetVolume(0.3f, false);
	titleBGM->SetAudioName("TitleBGM");
	AudioManager::Instance().Register(titleBGM);
	AudioManager::Instance().GetAudioResource("TitleBGM")->Play(true);

	AudioSource* decision = AudioManager::Instance().LoadAudioSource("./Resources/Audio/SE/Decision.wav", Audio::AudioType::SENormal, "TitleScene");
	decision->SetVolume(0.2f, false);
	decision->SetAudioName("Decision");
	AudioManager::Instance().Register(decision);

	//	スプライト初期化
	sprites_[static_cast<int>(SpriteTitle::Back)] = std::make_unique<Sprite>(L"./Resources/Image/Back2.png");
	sprites_[static_cast<int>(SpriteTitle::Groove)] = std::make_unique<Sprite>(L"./Resources/Image/Groove2.png");
	sprites_[static_cast<int>(SpriteTitle::KeyText)] = std::make_unique<Sprite>(L"./Resources/Image/KeyText2.png");
	sprites_[static_cast<int>(SpriteTitle::KeyText)]->GetTransform()->SetPosition(640, 855);

	//	ステート更新処理
	stateMachine_.reset(new StateMachine<State<SceneTitle>>());
	stateMachine_->RegisterState(new TitleState::FadeInState(this));	//	FadeIn
	stateMachine_->RegisterState(new TitleState::MainState(this));		//	Main
	stateMachine_->RegisterState(new TitleState::SettingState(this));	//	Setting
	//	初期ステート設定
	stateMachine_->SetState(static_cast<int>(StateType::FadeIn));		//	初期ステートセット

}

//	終了化
void SceneTitle::Finalize()
{
	//	スプライト終了化
	for (int i = 0; i < static_cast<int>(SpriteTitle::Max); i++)
	{
		if (sprites_[i] != nullptr)
		{
			sprites_[i] = nullptr;
		}
	}

	//  UI終了化
	UIManager::Instance().Finalize();

	//	オーディオ終了化
	AudioManager::Instance().RemoveByScene("TitleScene");

}

//	更新処理
void SceneTitle::Update(const float& elapsedTime)
{
	const GamePadButton anyButton =
		GamePad::BTN_A		//Z
		| GamePad::BTN_B	//X
		| GamePad::BTN_X	//C
		| GamePad::BTN_Y;	//V

	// ----- ステートマシン更新 -----
	stateMachine_->Update(elapsedTime);

}

//	ローディングシーンへ遷移 
void SceneTitle::ChangeLoadingScene()
{
	GamePad& gamePad = Input::Instance().GetGamePad();

	//	Aボタン(Zキー)が押されたらSEを鳴らし、Fadeステートへ遷移
	if (gamePad.GetButtonDown() & GamePad::BTN_A/*Zキー*/)
	{
		AudioManager::Instance().GetAudioResource("Decision")->Play(false);
		SceneManager::Instance().ChangeScene(new SceneLoading(new SceneGame));
	}
}

//	描画処理
void SceneTitle::Render()
{
	Graphics::Instance().GetShader()->SetDepthStencilState(Shader::DEPTH_STENCIL_STATE::ZT_ON_ZW_ON);
	Graphics::Instance().GetShader()->SetBlendState(Shader::BLEND_STATE::ALPHA);
	Graphics::Instance().GetShader()->SetRasterizerState(Shader::RASTERIZER_STATE::CULL_NONE);

	//	スプライト描画
	sprites_[static_cast<int>(SpriteTitle::Back)]->Render();
	sprites_[static_cast<int>(SpriteTitle::Groove)]->GetTransform()->SetColorA(titleLogoAlpha_);
	sprites_[static_cast<int>(SpriteTitle::Groove)]->Render();
	sprites_[static_cast<int>(SpriteTitle::KeyText)]->GetTransform()->SetColorA(keyTextAlpha_);
	sprites_[static_cast<int>(SpriteTitle::KeyText)]->Render();

}

//	現在のステート表示
void SceneTitle::DrawStateStr()
{
	//	ステート文字列
	std::string stateStr[static_cast<int>(StateType::Max)] =
	{
		"FadeIn","Main","Setting"
	};

	ImGui::Text(u8"State　%s", stateStr[static_cast<int>(stateMachine_->GetStateIndex())].c_str());	//	ステート表示

}

//	デバッグ描画
void SceneTitle::DrawDebug()
{
	//	----- ステート -----
	DrawStateStr();
	stateMachine_->DrawDebug();

	if (ImGui::TreeNode("Sprite"))	//	タイトル画面
	{
		if (ImGui::TreeNode("Back"))	//	タイトル画面
		{
			sprites_[static_cast<int>(SpriteTitle::Back)]->DrawDebug();
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("KeyText"))
		{
			sprites_[static_cast<int>(SpriteTitle::KeyText)]->DrawDebug();
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("TitleText"))
		{
			sprites_[static_cast<int>(SpriteTitle::Groove)]->DrawDebug();
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("UI"))
		{
			UIManager::Instance().DrawDebug();
			ImGui::TreePop();
		}
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Audio"))
	{
		//se_[static_cast<int>(AUDIO_SE_TITLE::Decision)]->DrawDebug();
		ImGui::TreePop();
	}
}



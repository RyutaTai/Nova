#include "TitleState.h"

#include "../Nova/Scenes/SceneManager.h"
#include "../Nova/Scenes/SceneTitle.h"
#include "../Nova/Scenes/SceneLoading.h"
#include "../Nova/Scenes/SceneGame.h"
#include "../Nova/Others/Easing.h"

//	FadeIn
namespace TitleState
{
	void FadeInState::Initialize()
	{
		logoAlpha_ = 0.0f;
	}

	void FadeInState::Update(const float& elapsedTime)
	{
		//  ----- タイトル文字のフェード -----
		logoAlpha_ = Easing::InSine(stateElapsedTime_, fadeTime_, 1.0f, 0.0f);

		//	ロゴのアルファ値が1.0f以上ならメインステートへ遷移
		if (logoAlpha_ >= 1.0f)
		{
			logoAlpha_ = 1.0f;
			owner_->ChangeState(SceneTitle::StateType::Main);
		}
		
		//	アルファ値を適応
		owner_->SetTitleLogoAlpha(logoAlpha_);
	}

	void FadeInState::Finalize()
	{

	}

	void FadeInState::DrawDebug()
	{
		if (ImGui::TreeNode("FadeInState"))
		{
			ImGui::DragFloat("StateElapsedTime", &stateElapsedTime_, 0.1f);

			ImGui::DragFloat("FadeTime", &fadeTime_, 0.01f);
			ImGui::DragFloat("LogoAlpha", &logoAlpha_, 0.01f);

			ImGui::TreePop();
		}
	}
}

//	Main
namespace TitleState
{
	void MainState::Initialize()
	{
		owner_->SetKeyTextAlpha(1.0f);
	}

	void MainState::Update(const float& elapsedTime)
	{
		//	ボタンが押されたらローディングシーンへ遷移
		owner_->ChangeLoadingScene();
	}

	void MainState::Finalize()
	{

	}

	void MainState::DrawDebug()
	{
		if (ImGui::TreeNode("MainState"))
		{
			ImGui::DragFloat("StateElapsedTime", &stateElapsedTime_, 0.1f);

			ImGui::TreePop();
		}
	}

}

//	Setting
namespace TitleState
{
	void SettingState::Initialize()
	{
		
	}

	void SettingState::Update(const float& elapsedTime)
	{
		
	}

	void SettingState::Finalize()
	{

	}

	void SettingState::DrawDebug()
	{
		if (ImGui::TreeNode("SettingState"))
		{
			ImGui::DragFloat("StateElapsedTime", &stateElapsedTime_, 0.1f);

			ImGui::TreePop();
		}
	}

}

#include "SceneManager.h"

#include "../Audio/AudioManager.h"
#include "../../imgui/imgui.h"

//	デストラクタ
SceneManager::~SceneManager()
{
	if (currentScene_)	//	現在のシーン終了化
	{
		delete currentScene_;
		currentScene_ = nullptr;
	}
	if (nextScene_)	//	次のシーン終了化
	{
		delete nextScene_;
		nextScene_ = nullptr;
	}
}

//	更新処理
void SceneManager::Update(const float& elapsedTime)
{
	//	シーン遷移
	if (nextScene_ != nullptr)//	次のシーンが設定されていたらクリア
	{
		Clear();
		currentScene_ = nextScene_;
		nextScene_ = nullptr;

		if (!currentScene_->IsReady())
		{
			currentScene_->Initialize();
		}
	}

	//	現在のシーンの更新処理
	if (currentScene_ != nullptr)
	{
		currentScene_->Update(elapsedTime);
		//	現在のシーンの経過時間更新
		currentSceneTime_ += elapsedTime;
	}
}

//	Shadow描画
void SceneManager::ShadowRender()
{
	if (currentScene_ != nullptr)
	{
		currentScene_->ShadowRender();
	}
}

//	描画処理
void SceneManager::Render()
{
	if (currentScene_ != nullptr)
	{
		currentScene_->Render();
	}
}

//	デバッグ描画
void SceneManager::DrawDebug()
{
#ifdef USE_IMGUI
	if (ImGui::TreeNode("SceneManager"))
	{
		ImGui::DragFloat("CurrentSceneTimer", &currentSceneTime_);
		ImGui::TreePop();
	}
	currentScene_->DrawDebug();
#endif// USE_IMGUI
}

//	シーンクリア
void SceneManager::Clear()
{
	if (currentScene_ != nullptr)
	{
		currentScene_->Finalize();
		delete currentScene_;
		currentScene_ = nullptr;
		currentSceneTime_ = 0.0f;
	}
}

//	シーン切り替え
void SceneManager::ChangeScene(Scene* scene)
{
	nextScene_ = scene;
}

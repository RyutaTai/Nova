#include "SceneLoading.h"

#include "SceneManager.h"
#include "../Graphics/Graphics.h"
#include "../Core/Framework.h"
#include "../Others/Transform.h"
#include "../Others/MathHelper.h"

//	初期化
void SceneLoading::Initialize()
{
	//	スプライト初期化
	sprites_[static_cast<int>(SpriteLoading::Loading)] = std::make_unique<Sprite>( L"./Resources/Image/Loading.png");
	sprites_[static_cast<int>(SpriteLoading::Loading)]->GetTransform()->SetPosition(900, 855);
	sprites_[static_cast<int>(SpriteLoading::Loading)]->GetTransform()->AddPosition(50, 50);
	sprites_[static_cast<int>(SpriteLoading::Loading)]->GetTransform()->SetPivot(0.5f, 0.5f);
	sprites_[static_cast<int>(SpriteLoading::Back)] = std::make_unique<Sprite>(L"./Resources/Image/Back2.png");
	sprites_[static_cast<int>(SpriteLoading::TitleText)] = std::make_unique<Sprite>( L"./Resources/Image/Groove2.png");

	//	スレッド開始
	thread_ = new std::thread(LoadingThread, this);	//	LoadingThread関数にthisを渡す

	//	ロード画像の角度初期化
	loadSpriteAngle_ = 0.0f;

	//	オーディオ初期化
	AudioSource* loadBGM = AudioManager::Instance().LoadAudioSource("./Resources/Audio/BGM/Load.wav", Audio::AudioType::BGMNormal, "LoadingScene");
	loadBGM->SetVolume(0.2f, false);
	loadBGM->SetAudioName("LoadBGM");
	AudioManager::Instance().Register(loadBGM);
	AudioManager::Instance().GetAudioResource("LoadBGM")->Play(true);

}

//	終了化
void SceneLoading::Finalize()
{
	//	スレッド終了化
	if (thread_ != nullptr)
	{
		thread_->join();	//	処理が終わるまで待つ
		delete thread_;
		thread_ = nullptr;
	}

	//	スプライト終了化
	for (int i = 0; i < static_cast<int>(SpriteLoading::Max); i++)
	{
		if (sprites_[i] != nullptr)
		{
			sprites_[i] = nullptr;
		}
	}

	//	オーディオ終了化
	AudioManager::Instance().RemoveByScene("LoadingScene");

}

//	更新処理
void SceneLoading::Update(const float& elapsedTime)
{
	//	次のシーンの準備が完了したらシーンを切り替える
	if (nextScene_->IsReady()) 
	{
		SceneManager::Instance().ChangeScene(nextScene_);
		nextScene_ = nullptr;
	}

	//	角度更新
	loadSpriteAngle_ += 90.0f * elapsedTime;
	if (loadSpriteAngle_ > 360.0f)
	{
		loadSpriteAngle_ = 0.0f;
	}
	sprites_[static_cast<int>(SpriteLoading::Loading)]->GetTransform()->SetAngle(loadSpriteAngle_);

}

//	タイトルシーンのスプライトのアルファ値を設定
void SceneLoading::SetAllSpriteAlpha()
{
	for (int i = 0; i < static_cast<int>(SpriteLoading::Max); ++i)
	{
		sprites_[i]->GetTransform()->SetColorA(allSpriteAlpha_);
	}
}

void SceneLoading::ShadowRender()
{

}

//	描画処理
void SceneLoading::Render()
{
	//	Sprite
	sprites_[static_cast<int>(SpriteLoading::Back)]->Render();
	sprites_[static_cast<int>(SpriteLoading::TitleText)]->Render();
	sprites_[static_cast<int>(SpriteLoading::Loading)]->Render();
	
}

//	ローディングスレッド
void SceneLoading::LoadingThread(SceneLoading* scene)
{
	//	COM関連の初期化でスレッド毎に呼ぶ必要がある
	CoInitialize(nullptr);

	//	次のシーンの初期化を行う
	scene->nextScene_->Initialize();

	//	スレッドが終わる前にCOM関連の終了化
	CoUninitialize();

	//	次のシーンの準備完了設定
	scene->nextScene_->SetReady();

}

//	デバッグ描画
void SceneLoading::DrawDebug()
{
	if (ImGui::TreeNode("Loading Sprite"))
	{
		for (int i = 0; i < static_cast<int>(SpriteLoading::Max); i++)
		{
			if (sprites_[i] != nullptr)
			{
				sprites_[i]->DrawDebug();
			}
		}
		ImGui::TreePop();
	}
}
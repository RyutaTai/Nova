#include "AudioManager.h"

#include "../Others/Misc.h"
#include "../../imgui/imgui.h"

void AudioManager::Initialize()
{
	HRESULT hr = S_OK;

	//	COMの初期化
	hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	UINT32 createFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	createFlags |= XAUDIO2_DEBUG_ENGINE;
#endif

	//	XAudio初期化
	hr = XAudio2Create(&xaudio_, createFlags);
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	//	create masteringvoice
#if 0 //	自動検出
	hr = xaudio->CreateMasteringVoice(&masteringVoice, XAUDIO2_DEFAULT_CHANNELS, 44100/*サンプリングレート*/, 0U, NULL, 0, AudioCategory_GameEffects);
#else
	hr = xaudio_->CreateMasteringVoice(&masteringVoice_, 2, 44100/*サンプリングレート*/, 0U, NULL, 0, AudioCategory_GameEffects);
#endif

	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	masteringVoice_->GetChannelMask(&channelMask_);

}

//	デストラクタ
AudioManager::~AudioManager()
{
	//	オーディオ全削除
	Finalize();

	//	マスタリングボイス破棄
	if (masteringVoice_ != nullptr)
	{
		masteringVoice_->DestroyVoice();
		masteringVoice_ = nullptr;
	}

	//	XAudio終了化
	if (xaudio_ != nullptr)
	{
		xaudio_->Release();
		xaudio_ = nullptr;
	}

	//	COM終了化
	CoUninitialize();
}

//	オーディオソース読み込み
AudioSource* AudioManager::LoadAudioSource(const char* filename, const Audio::AudioType& audioType, const std::string& sceneName)
{
	WaveReader* resource = new WaveReader(filename);
	return new AudioSource(xaudio_, resource, audioType, sceneName);
}

AudioSource3D* AudioManager::LoadAudioSource3D(const char* filename, const Audio::AudioType& audioType, const std::string& sceneName, SoundEmitter* emitter)
{
	WaveReader* resource = new WaveReader(filename);
	return new AudioSource3D(xaudio_, resource, audioType, sceneName, emitter);
}

//	更新処理
void AudioManager::Update(const float& elapsedTime)
{
#if 0
	//	破棄処理
	for (Audio* audio : audioRemoves_)
	{
		//	種類がSEかつオーディオ再生が終了していなかったら破棄しない。BGMは無条件に破棄
		if (audio->IsSE() && audio->GetState().BuffersQueued != 0)continue;

		std::vector<Audio*>::iterator it =
			std::find(audioResources_.begin(), audioResources_.end(), audio);

		if (it != audioResources_.end())
		{
			audioResources_.erase(it);
		}

		delete audio;
	}

	//	破棄リストをクリア
	audioRemoves_.clear();

	for (Audio* audio : audioResources_)
	{
		audio->Update(elapsedTime);
	}
#else
	//	破棄処理
	for (auto it = audioRemoves_.begin(); it != audioRemoves_.end();)
	{
		Audio* audio = *it;

		//	SEの場合、再生が終了していないなら破棄しない
		if (audio->IsSE() && audio->GetState().BuffersQueued != 0)
		{
			++it;  // 再生中なら破棄せず、次のオーディオへ
			continue;
		}

		//	BGMや再生終了したSEは破棄
		//	audioがaudioResources_内に存在するか確認
		auto audioIt = std::find(audioResources_.begin(), audioResources_.end(), audio);
		if (audioIt != audioResources_.end())
		{
			audioResources_.erase(audioIt);
			delete audio;
		}

		//	破棄したオーディオをリストから削除
		it = audioRemoves_.erase(it);
	}

	//	残りのオーディオを更新
	for (Audio* audio : audioResources_)
	{
		audio->Update(elapsedTime);
	}
#endif
}

//	オーディオ登録
void AudioManager::Register(Audio* audio)
{	
	audioResources_.emplace_back(audio);
}

//	オーディオを名前から取得(例: デフォルトならTitle.wavなど.wavまで含めた名前、SetAudioName()で設定した場合はその名前。)
Audio* AudioManager::GetAudioResource(const std::string& name)
{
	for (int i = 0; i < audioResources_.size(); ++i)
	{
		if (strcmp(audioResources_.at(i)->GetAudioName().c_str(), name.c_str()) == 0)	//	入力文字列と等しいデータがあれば
		{
			return audioResources_.at(i);
		}
	}
	_ASSERT_EXPR(false, L"Audio is not found.");
	return nullptr;
}

//	オーディオ削除
void AudioManager::Remove(Audio* audio)
{
	audioRemoves_.insert(audio);
}

//	シーンを指定してオーディオ削除
void AudioManager::RemoveByScene(const std::string& sceneName)
{
	for (Audio* audio : audioResources_)
	{
		if (strcmp(audio->GetSceneName().c_str(), sceneName.c_str()) == 0)	//	オーディオデータのシーンと一致したら
		{
			audioRemoves_.insert(audio);
		}
	}
}

//	オーディオ全削除
void AudioManager::Clear()
{
	for (Audio*& audio : audioResources_)
	{
		delete audio;
	}
	audioResources_.clear();
}

//	オーディオ終了化
void AudioManager::Finalize()
{
	Clear();
}

//	デバッグ描画
void AudioManager::DrawDebug()
{
	if (ImGui::TreeNode("AudioManager"))
	{
		int size = static_cast<int>(audioResources_.size());
		ImGui::DragInt("AudioCount", &size);	//	オーディオの数

		for (Audio* audio : audioResources_)	//	各オーディオのImGui
		{
			std::string name = audio->GetAudioName();
			if (ImGui::TreeNode(name.c_str()))
			{
				audio->DrawDebug();
				ImGui::TreePop();
			}
		}
		ImGui::TreePop();
	}
}
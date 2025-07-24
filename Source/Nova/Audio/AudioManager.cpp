#include "AudioManager.h"

#include <memory>

#include "../../imgui/imgui.h"
#include "../Graphics/Graphics.h"
#include "../Camera/Camera.h"
#include "../Others/Misc.h"
#include "../Others/MemoryUtility.h"
#include "../Others/Dialog.h"

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
	//	XAudioの初期化
	hr = XAudio2Create(&xaudio_, createFlags);
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	//	マスターボイスを作成
#if 0	//	自動検出で設定
	hr = xaudio->CreateMasteringVoice(&masteringVoice, XAUDIO2_DEFAULT_CHANNELS, 44100/*サンプリングレート*/, 0U, NULL, 0, AudioCategory_GameEffects);
#else	//	手動で設定	
	hr = xaudio_->CreateMasteringVoice(&masteringVoice_, 2, 44100/*サンプリングレート*/, 0U, NULL, 0, AudioCategory_GameEffects);
#endif
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	masteringVoice_->GetChannelMask(&channelMask_);

}

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

	//	XAudio終了化(マスターボイスより後)
	if (xaudio_ != nullptr)
	{
		xaudio_->Release();
		xaudio_ = nullptr;
	}

	//	COM終了化
	CoUninitialize();
}

//	オーディオソース読み込み
std::shared_ptr<AudioSource> AudioManager::LoadAudioSource(const char* filename, const Audio::AudioType& audioType, const std::string& sceneName)
{
	WaveReader* resource = new WaveReader(filename);
	return std::make_shared<AudioSource>(xaudio_, resource, audioType, sceneName);
}

//	3Dで鳴らすオーディオソース読み込み
std::shared_ptr<AudioSource3D> AudioManager::LoadAudioSource3D(const char* filename, const Audio::AudioType& audioType, const std::string& sceneName, SoundEmitter* emitter)
{
	WaveReader* resource = new WaveReader(filename);
	return std::make_shared<AudioSource3D>(xaudio_, resource, audioType, sceneName, emitter);
}

//	更新処理
void AudioManager::Update(const float& elapsedTime)
{
	//	破棄処理
	//for (auto it = audioRemoves_.begin(); it != audioRemoves_.end();)
	//{
	//	Audio* audioToRemove = *it;

	//	//	SEの場合、再生が終了していないなら破棄しない
	//	if (audioToRemove->IsSE() && audioToRemove->GetState().BuffersQueued != 0)
	//	{
	//		++it;  // 再生中なら破棄せず、次のオーディオへ
	//		continue;
	//	}

	//	//	BGMや再生終了したSEは破棄
	//	//	audioがaudioResources_内に存在するか確認
	//	auto audioIt = std::find(audioResources_.begin(), audioResources_.end(), audioToRemove);
	//	if (audioIt != audioResources_.end())
	//	{
	//		audioResources_.erase(audioIt);
	//		SafeDelete(audioToRemove);
	//	}

	//	//	破棄したオーディオをリストから削除
	//	it = audioRemoves_.erase(it);
	//}

	//	残りのオーディオを更新
	for (const auto& audio : audioResources_)
	{
		if(auto a = audio.lock())
			a->Update(elapsedTime);
	}

}

//	オーディオ登録
void AudioManager::AudioRegister(std::shared_ptr<Audio> audio)
{
	audioResources_.emplace_back(audio);
}

//	リスナー登録
void AudioManager::ListenerRegister(std::shared_ptr<SoundListener> listener)
{
	soundListeners_.emplace_back(listener);
}

//	エミッター登録
void AudioManager::EmitterRegister(std::shared_ptr<SoundEmitter> emitter)
{
	soundEmitters_.emplace_back(emitter);
}

//	名前で指定して再生する
void AudioManager::PlayAudioByName(const std::string& audioName, const bool& loop)
{
	for (int i = 0; i < static_cast<int>(audioResources_.size()); ++i)
	{
		//	入力文字列と等しいデータがあれば
		if (auto resource = audioResources_.at(i).lock())
		{
			if (strcmp(resource->GetAudioName().c_str(), audioName.c_str()) == 0)
			{
				resource->Play(loop);
				return;
			}
		}
	}
	_ASSERT_EXPR(false, L"AudioResource is not found.");
}

std::shared_ptr<Audio>	AudioManager::GetAudioResource(const int& index)
{
	return audioResources_.at(index).lock(); 
}

//	オーディオを名前から取得(例: デフォルトならTitle.wavなど.wavまで含めた名前、SetAudioName()で設定した場合はその名前。)
std::shared_ptr<Audio> AudioManager::GetAudioResource(const std::string& name)
{
	for (const auto& audio : audioResources_)
	{
		//	入力文字列と等しいデータがあれば
		if (auto a = audio.lock())
		{
			if (strcmp(a->GetAudioName().c_str(), name.c_str()) == 0)
			{
				return a;
			}
		}
	}
	_ASSERT_EXPR(false, L"AudioResource is not found.");
	return nullptr;
}

//	指定したオーディオが存在するか
const bool AudioManager::AudioSourceIsExist(const std::string& name)const
{
	for (int i = 0; i < static_cast<int>(audioResources_.size()); ++i)
	{
		//	入力文字列と等しいデータがあれば
		if (auto resource = audioResources_[i].lock())
		{
			if (strcmp(resource->GetAudioName().c_str(), name.c_str()) == 0)
			{
				return true;
			}
		}
	}
	return false;
}

//	オーディオ再生フラグ設定
void AudioManager::SetAllPlayableFlag(const bool& isPlayable)
{
	for (const auto& audio : audioResources_)
	{
		if (auto resource = audio.lock())
		{
			resource->SetPlayable(isPlayable);
			//	false→trueになった場合リスタート
			if (resource->IsPlayable())
			{
				resource->Restart();
			}
			//	true→falseになった場合一時停止
			else
			{
				resource->Pause();
			}
		}
	}
}
//
////	オーディオ削除
//void AudioManager::Remove(Audio* audio)
//{
//	audioRemoves_.insert(audio);
//}
//
////	シーンを指定してオーディオ削除
//void AudioManager::RemoveBySceneName(const std::string& sceneName)
//{
//	for (const auto& audio : audioResources_)
//	{
//		if (strcmp(audio->GetSceneName().c_str(), sceneName.c_str()) == 0)	//	オーディオデータのシーンと一致したら
//		{
//			audioRemoves_.insert(audio.get());
//		}
//	}
//}

//	オーディオ全削除
void AudioManager::Clear()
{
	//	オーディオリソースクリア
	audioResources_.clear();

	//	エミッタークリア
	soundEmitters_.clear();

	//	リスナークリア
	soundListeners_.clear();

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
		//	オーディオの数
		int size = static_cast<int>(audioResources_.size());
		ImGui::DragInt("AudioCount", &size);
		
		//	オーディオ再生フラグ
		if (ImGui::Checkbox("IsAllPlayable", &isAllPlayable_))
		{
			SetAllPlayableFlag(isAllPlayable_);
		}

		//	各オーディオのImGui
		int audioIndex = 0;
		for (const auto& audio : audioResources_)
		{
			if (auto resource = audio.lock())
			{
				std::string name = resource->GetAudioName();
				if (ImGui::TreeNode(name.c_str()))
				{
					//	オーディオファイル選択メニュー表示
					DrawAudioSelection(audioIndex);
					//	オーディオのImGui描画
					resource->DrawDebug();
					ImGui::TreePop();
				}
			}
			audioIndex++;
		}
		ImGui::TreePop();
	}
}

//	オーディオファイル選択メニュー表示
void AudioManager::DrawAudioSelection(const int& audioIndex)
{
	//	ファイル選択ボタン
	if (ImGui::TreeNode(u8"Audio File Select"))
	{
		//	----- オーディオの種類を選択 -----
		char* audioTypeNames[] =
		{
			"BGMNormal","SENormal","BGM3D","SE3D",
		};

		int audioTypeId = 0;
		if (auto resource = audioResources_[audioIndex].lock())
		{
			audioTypeId = resource->GetAudioTypeID();
			ImGui::Combo("Audio Type", &audioTypeId, audioTypeNames, _countof(audioTypeNames));
			//	audioResource.at(audioIndex).GetAudioType()にしないのは、ImGuiで変更したaudioTypeを使用したいから

			Audio::AudioType audioType;
			switch (audioTypeId)
			{
			case 0: audioType = Audio::AudioType::BGMNormal; break;
			case 1: audioType = Audio::AudioType::SENormal;	 break;
			case 2: audioType = Audio::AudioType::BGM3D;	 break;
			case 3: audioType = Audio::AudioType::SE3D;		 break;
			}

			//	----- シーン名を選択 -----
			const char* sceneNames[] =
			{
				"TitleScene","LoadingScene","GameScene",
			};
			int sceneTypeId = resource->GetSceneTypeID();
			ImGui::Combo("Scene Type", &sceneTypeId, sceneNames, _countof(sceneNames));
			resource->SetSceneTypeID(sceneTypeId);

			//	----- 3Dの場合リスナーとエミッターを設定する -----
			//	立体音響を使う音源なら
			if (audioTypeId >= 2)
			{
				//	リスナーを選択                                                                                                                                                                                                                    
				std::string currentListenerName = resource->GetListenerName();
				if (ImGui::BeginCombo("Listener Select%s", currentListenerName.c_str()))
				{
					for (int i = 0; i < soundListeners_.size(); ++i)
					{
						if (auto listener = soundListeners_.at(i).lock())
						{
							const bool isSelected = (currentListenerName == listener->name_.c_str());
							if (ImGui::Selectable(listener->name_.c_str(), isSelected))
							{
								currentListenerName = listener->name_.c_str();
								resource->SetListenerName(currentListenerName);
							}
							if (isSelected)
								ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}

				//	エミッターを選択
				std::string currentEmitterName = resource->GetEmitterName();
				if (ImGui::BeginCombo(u8"Emitter Select%s", currentEmitterName.c_str()))
				{
					for (int i = 0; i < soundEmitters_.size(); ++i)
					{
						if (auto emitter = soundEmitters_[i].lock())
						{
							const bool isSelected = (currentEmitterName == emitter->name_.c_str());
							if (ImGui::Selectable(emitter->name_.c_str(), isSelected))
							{
								currentEmitterName = emitter->name_.c_str();
								resource->SetEmitterName(currentEmitterName);
							}
							if (isSelected)
								ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}
			}

			//	----- オーディオファイルを選択し、新しく読み込む -----
			if (ImGui::Button("Open AudioFile"))
			{
				const char* filter = "Audio Files(*.wav;)\0*.wav;\0All Files(*.*)\0*.*;\0\0";

				char filename[256] = "./Resources/Audio/";
				HWND hWnd = Graphics::Instance().GetWindowHandle();
				DialogResult result = Dialog::OpenFileName(filename, sizeof(filename), filter, nullptr, hWnd);
				if (result == DialogResult::OK)
				{
					//	新しくオーディオを読み込む
					//	立体音響なし
					if (audioTypeId <= 1)
					{
						//	前フレームの必要な情報を保存し、再生中なら停止しておく
						std::string lastAudioName = resource->GetAudioName();	//	前フレームの音源名を保存
						bool		lastPlayFlag = resource->IsPlaying();		//	前のフレームの再生フラグを保存
						bool		lastIsLoopFlag = resource->GetIsLoopFlag();	//	前のフレームの音源のループ設定保存
						if (lastPlayFlag)resource->Stop();							//	再生中なら停止する

						//	オーディオファイルを新しく読み込む
						resource = AudioManager::Instance().LoadAudioSource(filename, audioType, sceneNames[sceneTypeId]);

						//	前フレームの情報を復元し、再生処理
						resource->SetAudioName(lastAudioName);				//	前フレームの音源名を復元
						if (lastPlayFlag)resource->Play(lastIsLoopFlag);		//	前フレームで再生していたなら再生再開

					}
					//	立体音響あり
					else
					{
						//	前フレームの必要な情報を保存し、再生中なら停止しておく
						std::string lastAudioName = resource->GetAudioName();	//	前フレームの音源名を保存
						bool lastPlayFlag = resource->IsPlaying();			//	前のフレームの再生フラグを保存
						bool lastIsLoopFlag = resource->GetIsLoopFlag();		//	前のフレームの音源のループ設定保存
						if (lastPlayFlag)resource->Stop();					//	再生中なら停止する

						//	選択されたリスナー番号を取得
						int listenerId = 0;
						for (const auto& listener : soundListeners_)
						{
							if (auto l = listener.lock())
							{
								if (l->name_ == resource->GetListenerName())
									break;
							}
							listenerId++;
						}

						//	選択されたエミッター番号を取得
						int emitterId = 0;
						for (const auto& emitter : soundEmitters_)
						{
							if (auto e = emitter.lock())
							{
								if (e->name_ == resource->GetEmitterName())
									break;
							}
							emitterId++;
						}
						//	新しく読み込む
						if (auto emitter = soundEmitters_.at(emitterId).lock())
						{
							resource = AudioManager::Instance().LoadAudioSource3D(filename, audioType, sceneNames[sceneTypeId], emitter.get());
						}
						if (auto listener = soundListeners_.at(listenerId).lock())
						{
							static_cast<AudioSource3D*>(resource.get())->SetDSPSetting(listener.get());
							static_cast<AudioSource3D*>(resource.get())->SetListenerName(listener->name_);
						}

						//	前フレームの情報を復元し、再生処理
						resource->SetAudioName(lastAudioName);				//	前フレームの音源名を復元
						if (lastPlayFlag)resource->Play(lastIsLoopFlag);	//	前フレームで再生していたなら再生再開
					}
				}
			}
		}
		ImGui::TreePop();
	}
}
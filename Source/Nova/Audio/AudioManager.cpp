#include "AudioManager.h"

#include <memory>

#include "../Graphics/Graphics.h"
#include "../Camera/Camera.h"
#include "../Others/Misc.h"
#include "../Others/MemoryUtility.h"
#include "../Others/Dialog.h"
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
AudioSource* AudioManager::LoadAudioSource(const char* filename, const Audio::AudioType& audioType, const std::string& sceneName)
{
	WaveReader* resource = new WaveReader(filename);
	return new AudioSource(xaudio_, resource, audioType, sceneName);
}

//	3Dで鳴らすオーディオソース読み込み
AudioSource3D* AudioManager::LoadAudioSource3D(const char* filename, const Audio::AudioType& audioType, const std::string& sceneName, SoundEmitter* emitter)
{
	WaveReader* resource = new WaveReader(filename);
	return new AudioSource3D(xaudio_, resource, audioType, sceneName, emitter);
}

//	更新処理
void AudioManager::Update(const float& elapsedTime)
{
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
			SafeDelete(audio);
		}

		//	破棄したオーディオをリストから削除
		it = audioRemoves_.erase(it);
	}

	//	残りのオーディオを更新
	for (Audio* audio : audioResources_)
	{
		audio->Update(elapsedTime);
	}

}

//	オーディオ登録
void AudioManager::AudioRegister(Audio* audio)
{
	audioResources_.emplace_back(audio);
}

//	リスナー登録
void AudioManager::ListenerRegister(SoundListener* listener)
{
	soundListeners_.emplace_back(listener);
}

//	エミッター登録
void AudioManager::EmitterRegister(SoundEmitter* emitter)
{
	soundEmitters_.emplace_back(emitter);
}

//	名前で指定して再生する
void AudioManager::PlayAudioByName(const std::string& audioName, const bool& loop)
{
	for (int i = 0; i < static_cast<int>(audioResources_.size()); ++i)
	{
		//	入力文字列と等しいデータがあれば
		if (strcmp(audioResources_.at(i)->GetAudioName().c_str(), audioName.c_str()) == 0)
		{
			audioResources_.at(i)->Play(loop);
			return;
		}
	}
	_ASSERT_EXPR(false, L"AudioResource is not found.");
}

//	オーディオを名前から取得(例: デフォルトならTitle.wavなど.wavまで含めた名前、SetAudioName()で設定した場合はその名前。)
Audio* AudioManager::GetAudioResource(const std::string& name)
{
	for (int i = 0; i < static_cast<int>(audioResources_.size()); ++i)
	{
		//	入力文字列と等しいデータがあれば
		if (strcmp(audioResources_.at(i)->GetAudioName().c_str(), name.c_str()) == 0)
		{
			return audioResources_.at(i);
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
		if (strcmp(audioResources_.at(i)->GetAudioName().c_str(), name.c_str()) == 0)
		{
			return true;
		}
	}
	return false;
}

//	オーディオ再生フラグ設定
void AudioManager::SetAllPlayableFlag(const bool& isPlayable)
{
	for (Audio* audio : audioResources_)
	{
		audio->SetPlayable(isPlayable);
		//	false→trueになった場合リスタート
		if (audio->IsPlayable())
		{
			audio->Restart();
		}
		//	true→falseになった場合一時停止
		else
		{
			audio->Pause();
		}
	}
}

//	オーディオ削除
void AudioManager::Remove(Audio* audio)
{
	audioRemoves_.insert(audio);
}

//	シーンを指定してオーディオ削除
void AudioManager::RemoveBySceneName(const std::string& sceneName)
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
	//	オーディオリソースクリア
	for (Audio*& audio : audioResources_)
	{
		delete audio;
	}
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
		for (Audio* audio : audioResources_)
		{
			std::string name = audio->GetAudioName();
			if (ImGui::TreeNode(name.c_str()))
			{
				//	オーディオファイル選択メニュー表示
				DrawAudioSelection(audioIndex);
				//	オーディオのImGui描画
				audio->DrawDebug();
				ImGui::TreePop();
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
		int audioTypeId = audioResources_.at(audioIndex)->GetAudioTypeID();
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
		audioResources_.at(audioIndex)->SetAudioTypeID(audioTypeId);

		//	----- シーン名を選択 -----
		const char* sceneNames[] = 
		{
			"TitleScene","LoadingScene","GameScene",
		};
		int sceneTypeId = audioResources_.at(audioIndex)->GetSceneTypeID();
		ImGui::Combo("Scene Type", &sceneTypeId, sceneNames, _countof(sceneNames));
		audioResources_.at(audioIndex)->SetSceneTypeID(sceneTypeId);

		//	----- 3Dの場合リスナーとエミッターを設定する -----
		//	立体音響を使う音源なら
		if (audioTypeId >= 2)
		{
			//	リスナーを選択                                                                                                                                                                                                                    
			std::string currentListenerName = audioResources_.at(audioIndex)->GetListenerName();
			if (ImGui::BeginCombo("Listener Select%s", currentListenerName.c_str()))
			{
				for (int i = 0; i < soundListeners_.size(); ++i)
				{
					const bool isSelected = (currentListenerName == soundListeners_[i]->name_.c_str());
					if (ImGui::Selectable(soundListeners_[i]->name_.c_str(), isSelected))
					{
						currentListenerName = soundListeners_[i]->name_.c_str();
						audioResources_.at(audioIndex)->SetListenerName(currentListenerName);
					}
					if (isSelected)
						ImGui:: SetItemDefaultFocus();
				}                                                                                                                                                                                                                                                                                                                                                                                                                                                   
				ImGui::EndCombo();
			}

			//	エミッターを選択
			std::string currentEmitterName = audioResources_.at(audioIndex)->GetEmitterName();
			if (ImGui::BeginCombo(u8"Emitter Select%s", currentEmitterName.c_str()))
			{
				for (int i = 0; i < soundEmitters_.size(); ++i)
				{
					const bool isSelected = (currentEmitterName == soundEmitters_[i]->name_.c_str());
					if (ImGui::Selectable(soundEmitters_[i]->name_.c_str(), isSelected))
					{
						currentEmitterName = soundEmitters_[i]->name_.c_str();
						audioResources_.at(audioIndex)->SetEmitterName(currentEmitterName);
					}
					if (isSelected)
						ImGui::SetItemDefaultFocus();
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
					std::string lastAudioName		= audioResources_.at(audioIndex)->GetAudioName();	//	前フレームの音源名を保存
					bool		lastPlayFlag		= audioResources_.at(audioIndex)->IsPlaying();		//	前のフレームの再生フラグを保存
					bool		lastIsLoopFlag		= audioResources_.at(audioIndex)->GetIsLoopFlag();	//	前のフレームの音源のループ設定保存
					if (lastPlayFlag)audioResources_.at(audioIndex)->Stop();							//	再生中なら停止する
					
					//	オーディオファイルを新しく読み込む
					audioResources_.at(audioIndex)	= AudioManager::Instance().LoadAudioSource(filename,audioType, sceneNames[sceneTypeId]);
					
					//	前フレームの情報を復元し、再生処理
					audioResources_.at(audioIndex)->SetAudioName(lastAudioName);				//	前フレームの音源名を復元
					if (lastPlayFlag)audioResources_.at(audioIndex)->Play(lastIsLoopFlag);		//	前フレームで再生していたなら再生再開

				}
				//	立体音響あり
				else
				{
					//	前フレームの必要な情報を保存し、再生中なら停止しておく
					std::string lastAudioName = audioResources_.at(audioIndex)->GetAudioName();	//	前フレームの音源名を保存
					bool lastPlayFlag = audioResources_.at(audioIndex)->IsPlaying();			//	前のフレームの再生フラグを保存
					bool lastIsLoopFlag = audioResources_.at(audioIndex)->GetIsLoopFlag();		//	前のフレームの音源のループ設定保存
					if (lastPlayFlag)audioResources_.at(audioIndex)->Stop();					//	再生中なら停止する

					AudioSource3D* audioSource3D = nullptr;
					//	選択されたリスナー番号を取得
					int listenerId = 0;
					for (const auto& listener : soundListeners_)
					{
						if (listener->name_ == audioResources_.at(audioIndex)->GetListenerName())
							break;
						listenerId++;
					}

					//	選択されたエミッター番号を取得
					int emitterId = 0;
					for (const auto& emitterPtr : soundEmitters_)
					{
						if (emitterPtr->name_ == audioResources_.at(audioIndex)->GetEmitterName())
							break;
						emitterId++;
					}
					//	新しく読み込む
					audioSource3D = AudioManager::Instance().LoadAudioSource3D(filename, audioType, sceneNames[sceneTypeId], soundEmitters_.at(emitterId).get());
					audioSource3D->SetDSPSetting(soundListeners_.at(listenerId).get());
					audioSource3D->SetListenerName(soundListeners_.at(listenerId)->name_);
					audioResources_.at(audioIndex) = audioSource3D;

					//	前フレームの情報を復元し、再生処理
					audioResources_.at(audioIndex)->SetAudioName(lastAudioName);				//	前フレームの音源名を復元
					if (lastPlayFlag)audioResources_.at(audioIndex)->Play(lastIsLoopFlag);		//	前フレームで再生していたなら再生再開
				}
			}
		}
		ImGui::TreePop();
	}
}
#pragma once

#include <xaudio2.h>
#include <set>

#include "AudioSource3D.h"
#include "AudioSource.h"

class AudioManager
{
private:
	AudioManager() {}
	~AudioManager();

public:
	static AudioManager& Instance()
	{
		static AudioManager instance;
		return instance;
	}

	void Initialize();						//	初期化処理
	void Update(const float& elapsedTime);	//	更新処理

	//	オーディオソース読み込み
	std::shared_ptr<AudioSource>	LoadAudioSource(const char* filename, const Audio::AudioType& audioType, const std::string& sceneName);
	std::shared_ptr<AudioSource3D>	LoadAudioSource3D(const char* filename, const Audio::AudioType& audioType, const std::string& sceneName, SoundEmitter* emitter);

	//	登録関数
	void AudioRegister(std::shared_ptr<Audio> audio);					//	オーディオ登録
	void ListenerRegister(std::shared_ptr<SoundListener> listener);		//	リスナー登録
	void EmitterRegister(std::shared_ptr <SoundEmitter> emitter);		//	エミッター登録

	//	オーディオ再生
	void PlayAudioByName(const std::string& audioName, const bool& loop);

	//	デバッグ描画
	void DrawDebug();

	IXAudio2* GetXAudio() { return xaudio_; }
	IXAudio2MasteringVoice* GetMasteringVoice() { return masteringVoice_; }
	DWORD					GetCannelmask() const { return channelMask_; }

	//	オーディオソース取得
	std::shared_ptr<Audio>	GetAudioResource(const int& index);
	std::shared_ptr<Audio>	GetAudioResource(const std::string& name);

	//	指定したオーディオソースが存在するか
	const bool AudioSourceIsExist(const std::string& name)const;

	//	----- 削除、終了化 -----
	void Finalize();						//	オーディオ終了化
	void Clear();							//	全削除
	//void Remove(Audio* audio);			//	オーディオ削除
	//void RemoveBySceneName(const std::string& sceneName);	//	オーディオをシーンごとに削除
	
	//	オーディオ再生フラグ設定
	void SetAllPlayableFlag(const bool& isPlayable);

	//	オーディオファイル選択メニュー表示
	void DrawAudioSelection(const int& audioIndex);

private:
	DWORD		channelMask_ = {};
	IXAudio2*	xaudio_ = nullptr;
	IXAudio2MasteringVoice* masteringVoice_ = nullptr;

	std::vector<std::weak_ptr<Audio>>			audioResources_ = {};	//	オーディオリソース
	//std::set<std::shared_ptr<Audio> audioRemoves_ = {};	//	オーディオ破棄リスト
	std::vector<std::weak_ptr<SoundEmitter>>	soundEmitters_	= {};	//	エミッター
	std::vector<std::weak_ptr<SoundListener>>	soundListeners_ = {};	//	リスナー

	bool isAllPlayable_ = false;	//	オーディオ再生フラグ(オーディオ全体)

};


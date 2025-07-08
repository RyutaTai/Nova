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
	AudioSource* LoadAudioSource(const char* filename, const Audio::AudioType& audioType, const std::string& sceneName);
	AudioSource3D* LoadAudioSource3D(const char* filename, const Audio::AudioType& audioType, const std::string& sceneName, SoundEmitter* emitter);

	//	登録関数
	void AudioRegister(Audio* audio);					//	オーディオ登録
	void ListenerRegister(SoundListener* listener);		//	リスナー登録
	void EmitterRegister(SoundEmitter* emitter);		//	エミッター登録


	//	オーディオ再生
	void PlayAudioByName(const std::string& audioName, const bool& loop);

	//	デバッグ描画
	void DrawDebug();

	IXAudio2* GetXAudio() { return xaudio_; }
	IXAudio2MasteringVoice* GetMasteringVoice() { return masteringVoice_; }
	DWORD					GetCannelmask() const { return channelMask_; }

	//	オーディオソース取得
	Audio*				GetAudioResource(const int& index) { return audioResources_.at(index); }
	Audio*				GetAudioResource(const std::string& name);
	std::vector<Audio*>	GetAudioResources() { return audioResources_; }

	//	指定したオーディオソースが存在するか
	const bool AudioSourceIsExist(const std::string& name)const;

	//	----- 削除、終了化 -----
	void Finalize();						//	オーディオ終了化
	void Clear();							//	全削除
	void Remove(Audio* audio);				//	オーディオ削除
	void RemoveBySceneName(const std::string& sceneName);	//	オーディオをシーンごとに削除
	
	//	オーディオ再生フラグ設定
	void SetAllPlayableFlag(const bool& isPlayable);

	//	オーディオファイル選択メニュー表示
	void DrawAudioSelection(const int& audioIndex);

private:
	DWORD		channelMask_ = {};
	IXAudio2*	xaudio_ = nullptr;
	IXAudio2MasteringVoice* masteringVoice_ = nullptr;

	std::vector<Audio*>			audioResources_ = {};	//	オーディオリソース
	std::set<Audio*>			audioRemoves_	= {};	//	オーディオ破棄リスト
	std::vector<std::unique_ptr<SoundEmitter>>	soundEmitters_	= {};	//	エミッター
	std::vector<std::unique_ptr<SoundListener>>	soundListeners_ = {};	//	リスナー

	bool isAllPlayable_ = false;	//	オーディオ再生フラグ(オーディオ全体)


};


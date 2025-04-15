#pragma once

#define	 XAUDIO2_HELPER_FUNCTIONS
#include <xaudio2.h>
#include <x3daudio.h>

#include "WaveRead.h"

class Audio
{
public:
	enum class AudioType
	{
		BGMNormal,
		SENormal,
		BGM3D,
		SE3D,
		Max
	};

public:
	Audio(IXAudio2* xaudio, WaveReader* resource, const AudioType& audioType = {}, const std::string& sceneName = {});
	virtual ~Audio();

public:
	virtual void Update(const float& elapsedTime) = 0;
	virtual void DrawDebug();
	
	virtual void SetPitch(const float& pitch) = 0;

public:
	void Play(const bool& loop);	//	再生
	void Restart();					//	再開
	void Stop();					//	停止
	void Pause();					//	一時停止

	//	タイマー加算
	void AddPlayTimer(const float& elapsedTime) { timer_ += elapsedTime; }
	void AddTotalPlayTimer(const float& elapsedTime) { totalPlayTimer_ += elapsedTime; }

	IXAudio2SourceVoice*	GetSourceVoice()			{ return sourceVoice_; }
	XAUDIO2_VOICE_SENDS*	GetSfxSendList()			{ return &SFXSendList_; }
	XAUDIO2_VOICE_STATE		GetState()			const	{ return state_; }
	const WAVEFORMATEX&		GetWaveFormat()		const	{ return wfx_; }				//	WAVEフォーマット取得
	const BYTE*				GetAudioData()		const	{ return buffer_.pAudioData; }
	size_t					GetAudioBytes()		const	{ return buffer_.AudioBytes; }	//	バッファーのサイズ取得
	size_t					GetCurrentSample()	const;									//	現在の再生位置をサンプル単位で取得

	int						GetPlayLength()		const	{ return length_; }
	float					GetPlayLengthFloat()const	{ return lengthFloat_; }

	void					ResetPlayTimer()			{ timer_ = 0.0f; }
	float					GetPlayTimer()		const	{ return timer_; }
	float					GetTotalPlayTimer()	const	{ return totalPlayTimer_; }
	
	void					SetVolume(const float& volume, const bool& useDb);
	float					GetVolume()			const	{ return lastVolume_; }


	void			SetAudioType(const AudioType& type) { audioType_ = type; }
	AudioType		GetAudioType() { return audioType_; }
	
	bool			IsPlaying();	//	手動で切り替えているisPlaying_フラグより精度がいいが、毎フレームGetStateを呼びたくないため分けている
	bool			IsBGM();		//	BGMかどうか
	bool			IsSE();			//	SEかどうか

	void			SetAudioName(const std::string& audioName) { audioName_ = audioName; }
	std::string		GetAudioName()	const { return audioName_; }
	void			SetSceneName(const std::string& sceneName) { sceneName_ = sceneName; }
	std::string		GetSceneName() { return sceneName_; }

	void			SetPlayable(const bool& playable) { isPlayable_ = playable; }
	bool			IsPlayable() { return isPlayable_; }

protected:
	static constexpr float		DefaultSamplingRate = 44100.0f;		//	基本のサンプリングレート
	static constexpr int		OutputMatrixMax_ = 8;				//	出力マトリックス最大数

	IXAudio2SourceVoice*		sourceVoice_ = nullptr;	//	ソースボイス
	XAUDIO2_BUFFER				buffer_ = {};			//	バッファー
	WAVEFORMATEX				wfx_ = {};				//	WAVEフォーマット情報
	XAUDIO2_SEND_DESCRIPTOR		SFXSend_;				//	
	XAUDIO2_VOICE_SENDS			SFXSendList_ = {};		//	
	XAUDIO2_VOICE_STATE			state_ = {};			//	ボイスの状態
	XAUDIO2_FILTER_PARAMETERS	filterParameters_ = {};	//	フィルターのパラメータ

	float	timer_			= 0.0f;		//	現在の再生時間(音データのどこまで再生したか)
	float	totalPlayTimer_	= 0.0f;		//	今までの再生時間(ループしてもリセットしない)

	int		length_			= {};		//	音源の長さ
	float	lengthFloat_	= {};		//	音源の長さ

	float	lastVolume_		= {};		//	前フレーム時点でのボリューム : SetVolumeを使う前にこの値と比べる

	bool	isPlaying_		= false;	//	再生中かどうかのフラグ(playしたらtrue,stopしたらfalseにしている)
	AudioType	audioType_ = {};	//	オーディオタイプ
	std::string audioName_ = {};	//	音源の名前
	std::string	sceneName_ = {};	//	使用シーンを設定(Title,Gameなど)

private:	//	デバッグ用
	bool isPlayable_ = true;	//	再生可能かどうか(ImGuiで再生する音を絞るときに使う)

};


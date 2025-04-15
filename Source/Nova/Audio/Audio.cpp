#include "Audio.h"

#include "../../imgui/imgui.h"
#include "../Others/Misc.h"

//	コンストラクタ
Audio::Audio(IXAudio2* xaudio, WaveReader* resource, const AudioType& audioType, const std::string& sceneName)
{
	HRESULT hr = S_OK;

	wfx_ = resource->GetWaveForMatex();

	//	ソースボイスを生成
	hr = xaudio->CreateSourceVoice(
		&sourceVoice_,							//	ソースボイスのインスタンスを返す(必須)
		&wfx_,									//	ソースボイスへ渡すWAVEFORMATRIXの構造体を返す(必須)
		XAUDIO2_VOICE_USEFILTER,				//	ソースボイスへ使用する効果を指定する
		XAUDIO2_MAX_FREQ_RATIO					//	最大許容再生速度を指定 デフォルトで2.0f, 最大でXAUDIO2_MAX_FREQ_RATIO(1024.0f)まで設定可能
	);
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	length_ = resource->GetPlayLength();
	lengthFloat_ = resource->GetPlayLengthFLOAT();

	//	ソースボイスにデータを送信
	buffer_.pAudioData = resource->GetAudioData();
	buffer_.AudioBytes = resource->GetAudioBytes();
	buffer_.Flags = XAUDIO2_END_OF_STREAM;
	//buffer.PlayLength = resource->GetWaveFormat().nSamplesPerSec * resource->GetPlayLength();
	//buffer.PlayBegin = resource->GetWaveFormat().nSamplesPerSec * 30; // 再生開始位置の指定 サンプル単位(サンプリングレート * 秒数)で指定

	SFXSend_ = { 0, sourceVoice_ };
	SFXSendList_ = { 1, &SFXSend_ };

	audioName_ = resource->GetName();
	audioType_ = audioType;
	sceneName_ = sceneName;

}

//	デストラクタ
Audio::~Audio()
{
	if (sourceVoice_ != nullptr)
	{
		sourceVoice_->DestroyVoice();
	}

	delete buffer_.pAudioData;
}

//	再生
void Audio::Play(const bool& loop)
{
	//	再生可能でないなら処理しない
	if (isPlayable_ == false)return;

	buffer_.LoopCount = loop ? XAUDIO2_LOOP_INFINITE : 0;

	//	再生終わりを待たずに即時再生(要調整)
	sourceVoice_->Stop();
	sourceVoice_->FlushSourceBuffers();

	sourceVoice_->SubmitSourceBuffer(&buffer_);
	HRESULT hr = sourceVoice_->Start();
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	timer_ = 0.0f;
	isPlaying_ = true;
}

//	再生再開
void Audio::Restart()
{
	//	再生可能でないなら処理しない
	if (isPlayable_ == false)return;

	sourceVoice_->Start();
	isPlaying_ = true;
	//_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	//sourceVoice->SetVolume(volume);
}

//	停止
void Audio::Stop()
{
	HRESULT hr = S_OK;
	hr = sourceVoice_->Stop();
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	hr = sourceVoice_->FlushSourceBuffers();
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	state_.SamplesPlayed = 0;	//	必要?

	isPlaying_ = false;
}

//	一時停止
void Audio::Pause()
{
	sourceVoice_->Stop(XAUDIO2_PLAY_TAILS); // 引数にXAUDIO2_PLAY_TAILSを設定することでかけた効果(リバーブの残響など)を残す
	isPlaying_ = false;
}

//	現在の再生位置をサンプル単位で取得
size_t Audio::GetCurrentSample()const
{
	//	現在をサンプル単位で計算
	XAUDIO2_VOICE_STATE vs;
	sourceVoice_->GetState(&vs);

	return (size_t(vs.SamplesPlayed) * size_t(wfx_.nBlockAlign)) % buffer_.AudioBytes;
	//return (size_t(vs.SamplesPlayed) * size_t(wfx_.nBlockAlign)) % buffer_.AudioBytes;
}

//	再生中かどうか
bool Audio::IsPlaying()
{
	//	再生中かどうか lengthfFloat_(音源の長さ)より再生時間(timer_)が小さかったら再生中
	//return lengthFloat_ > timer_;

	sourceVoice_->GetState(&state_);

	return state_.BuffersQueued;
}

//	BGMかどうか
bool Audio::IsBGM()
{
	if (audioType_ == AudioType::BGMNormal || audioType_ == AudioType::BGM3D)
	{
		return true;
	}
	return false;
}

//	SEかどうか
bool Audio::IsSE()
{
	if (audioType_ == AudioType::SENormal || audioType_ == AudioType::SE3D)
	{
		return true;
	}
	return false;
}

//	ボリューム設定
void Audio::SetVolume(const float& volume, const bool& useDb)
{
	float setVolume = volume;
	if (useDb)
	{
		if (setVolume <= -40.0f)
			setVolume = 0.0f;
		else
			setVolume = XAudio2DecibelsToAmplitudeRatio(volume); // dBを0.0f～1.0fに変換
	}
	if (setVolume != lastVolume_)// SetVolumeが重い処理のため分岐させておく
	{
		sourceVoice_->SetVolume(setVolume);
		lastVolume_ = setVolume;
	}

}

//	デバッグ描画
void Audio::DrawDebug()
{
#ifdef USE_IMGUI
	bool isPlay = IsPlaying();
	if (ImGui::Checkbox("IsPlayable", &isPlayable_))		//	フラグが切り替わったらif文に入る		
	{
		//	false→trueになった場合リスタート
		if (IsPlayable())
		{
			Restart();
		}
		//	true→falseになった場合一時停止
		else
		{
			Pause();
		}
	}
	ImGui::Checkbox("IsPlaying", &isPlay);					//	再生中かどうか
	ImGui::DragFloat("PlayTimer", &timer_);					//	現在のループの再生時間
	ImGui::DragFloat("TotalPlayTimer", &totalPlayTimer_);	//	合計再生時間
#endif
}
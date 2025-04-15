#include "AudioSource.h"

#include "Audio.h"
#include "AudioManager.h"
#include "../Others/Misc.h"
#include "../Graphics/Graphics.h"

//	コンストラクタ
AudioSource::AudioSource(IXAudio2* xaudio, WaveReader* resource, const AudioType& audioType, const std::string& sceneName)
	:Audio(xaudio, resource, audioType, sceneName)
{
	delete resource;	//	メモリリーク防止
}

//	デストラクタ
AudioSource::~AudioSource()
{
	
}

//	更新処理
void AudioSource::Update(const float& elapsedTime)
{
	sourceVoice_->GetState(&state_);
	
	//if (state.BuffersQueued == 1)

	//	再生中でないなら処理しない
	if (isPlaying_ == false)return;

	//	再生可能でないなら処理しない
	if (IsPlayable() == false)return;

	//	音源データの長さを超えたら再生時間リセット
	float length = GetPlayLengthFloat();
	if (GetPlayTimer() >= length)
	{
		ResetPlayTimer();
	}

	if(state_.BuffersQueued != 0)
	{
		//DrawDebug();
		AddPlayTimer(elapsedTime);
		AddTotalPlayTimer(elapsedTime);
	}
	else
	{
		isPlaying_ = false;
	}

	//INT32 CurrentDiskReadBuffer = 0;
	//INT32 CurrentPosition = 0;
	//while (CurrentPosition < resource->GetWaveFormat().cbSize)
	//{
	//	DWORD cbValid = min(STREAMING_BUFFER_SIZE, resource->GetWaveFormat().cbSize - CurrentPosition);
	//	DWORD dwRead;
	//	if (0 == ReadFile(hFile, buffers[CurrentDiskReadBuffer], STREAMING_BUFFER_SIZE, &dwRead, &overlapped))
	//		hr = HRESULT_FROM_WIN32(GetLastError());
	//	Overlapped.Offset += cbValid;

	//	//update the file position to where it will be once the read finishes
	//	CurrentPosition += cbValid;

	//	DWORD NumberBytesTransferred;
	//	::GetOverlappedResult(hFile, &Overlapped, &NumberBytesTransferred, TRUE);

	//	while (state.BuffersQueued >= MAX_BUFFER_COUNT - 1)
	//	{
	//		WaitForSingleObject(Context.hBufferEndEvent, INFINITE);
	//	}

	//	buffer.AudioBytes = cbValid;
	//	buffer.pAudioData = buffers[CurrentDiskReadBuffer];
	//	if (CurrentPosition >= resource->GetWaveFormat().cbSize)
	//	{
	//		buffer.Flags = XAUDIO2_END_OF_STREAM;
	//	}
	//	source_voice->SubmitSourceBuffer(&buffer);

	//	CurrentDiskReadBuffer++;
	//	CurrentDiskReadBuffer %= MAX_BUFFER_COUNT;
	//}

}

//	ピッチ設定
void AudioSource::SetPitch(const float& pitch)
{
	sourceVoice_->SetFrequencyRatio(pitch);
}

//	パンニング
void AudioSource::SetStereoPan(const float& pan)
{
	FLOAT32 outputPan[8];
	for (int i = 0; i < OutputMatrixMax_; i++) outputPan[i] = 0;

	XAUDIO2_VOICE_DETAILS voiceDetails;
	sourceVoice_->GetVoiceDetails(&voiceDetails);

	XAUDIO2_VOICE_DETAILS masterDetails;
	AudioManager::Instance().GetMasteringVoice()->GetVoiceDetails(&masterDetails);

	FLOAT32 left{ 0.5f - pan * 0.5f };
	FLOAT32 right{ 0.5f + pan * 0.5f };

	switch (AudioManager::Instance().GetCannelmask())
	{
	case SPEAKER_MONO:
		outputPan[0] = 1.0;
		break;

	case SPEAKER_STEREO:
	case SPEAKER_2POINT1:
	case SPEAKER_SURROUND:
		outputPan[0] = left;
		outputPan[3] = right;
		break;

	case SPEAKER_QUAD:
		outputPan[0] = outputPan[2] = left;
		outputPan[1] = outputPan[3] = right;
		break;

	case SPEAKER_4POINT1:
		outputPan[0] = outputPan[3] = left;
		outputPan[1] = outputPan[4] = right;
		break;

	case SPEAKER_5POINT1:
	case SPEAKER_7POINT1:
	case SPEAKER_5POINT1_SURROUND:
		outputPan[0] = outputPan[4] = left;
		outputPan[1] = outputPan[5] = right;
		break;

	case SPEAKER_7POINT1_SURROUND:
		outputPan[0] = outputPan[4] = outputPan[6] = left;
		outputPan[1] = outputPan[5] = outputPan[7] = right;
		break;
	}

	sourceVoice_->SetOutputMatrix(AudioManager::Instance().GetMasteringVoice(), voiceDetails.InputChannels, masterDetails.InputChannels, outputPan);

}

//	フィルター処理
void AudioSource::Filter(const XAUDIO2_FILTER_TYPE& type, const float& cutoff, const float& overq)
{
	filterParameters_.Type = type;				//	使うフィルターの種類
	filterParameters_.Frequency					//	カットする周波数の基準(0Hz(0.0f) ~ 7350Hz(1.0f))
		= cutoff / wfx_.nSamplesPerSec * 6.0f;	//	式:カットオフ周波数 / サンプリングレート * 6.0f 例 : 7350 / 44100 * 6.0f = 1.0f(正確には1.000002fだがまあこれでおｋ)
												//	単極フィルターを使う場合はXAudio2CutoffFrequencyToOnePoleCoefficient()というマクロを使う(XAUDIO2_HELPER_FUNCTIONSが必要)
	
	filterParameters_.OneOverQ = overq;			//	実際にどのくらいの音量がカットされているかを指定する

	sourceVoice_->SetFilterParameters(&filterParameters_);

}

//	デバッグ描画
void AudioSource::DrawDebug()
{
	Audio::DrawDebug();
}

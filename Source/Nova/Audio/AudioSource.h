#pragma once

#include <memory>
#include <string>

#include "Audio.h"

//	オーディオソース
class AudioSource :public Audio
{
public:
	AudioSource(IXAudio2* xaudio, WaveReader* resource, const AudioType& audioType, const std::string& sceneName);
	~AudioSource()override;

public:
	//	更新処理
	void Update(const float& elapsedTime)override;

	//	フィルター
	void Filter(const XAUDIO2_FILTER_TYPE& type, const float& cutoff = 7350.0f, const float& overq = 1.0f);

	void DrawDebug()override;

	void	SetPitch(const float& pitch)override;
	void	SetStereoPan(const float& pan);

};
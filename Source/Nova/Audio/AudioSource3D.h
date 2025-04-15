#pragma once

#include "Audio.h"
#include "Audio3DSystem.h"

class AudioSource3D : public Audio
{
public:
	AudioSource3D(IXAudio2* xaudio, WaveReader* resource, const AudioType& audioType, const std::string& sceneName, SoundEmitter* emitter);
	~AudioSource3D()override;

public:
	void Update(const float& elapsedTime)override;
		
	void DrawDebug()override;

public:
	void SetDSPSetting(SoundListener& listner)
	{
		DSP(dspSetting_, listner, *emitter_);
	}

	void SetEmitter(SoundEmitter emitter) { this->emitter_ = &emitter; }

	void SetPosition(const DirectX::XMFLOAT3& position) { emitter_->position_ = position; }

	void SetVelocity(const DirectX::XMFLOAT3& velocity) { emitter_->velocity_ = velocity; }

	void SetCurveDistanceScaler(const float& scaler) { emitter_->maxDistance_ = scaler; }

	// ドップラー効果を適用
	void SetPitch(const float& pitch) override { sourceVoice_->SetFrequencyRatio(dspSetting_.dopplerScale_ * pitch); }

	void Set3DPan();	//	3Dパンニング

	void SetReflectionRate(const float& reflectionRate) { this->reflectionRate_ = reflectionRate; }

	void SetAbsortRate(const float& absorptionRate) { this->absorptionRate_ = absorptionRate; }

	/*void SetVolumeCurve(X3DAUDIO_DISTANCE_CURVE_POINT vol_curvepoint[], UINT32 array_size)
	{
		if (array_size > 0)
		{
			this->vol_curvepoints.resize(array_size);
			for (uint32_t i = 0; i < array_size; ++i)
			{
				this->vol_curvepoints.at(i) = vol_curvepoint[i];
			}
			vol_curve = { &this->vol_curvepoints.at(0), array_size };
			emitter->pVolumeCurve = &vol_curve;
		}
	}

	void SetLPFDirectCurve(X3DAUDIO_DISTANCE_CURVE_POINT lpfdirect_curvepoint[], UINT32 array_size)
	{
		if (array_size)
		{
			this->lpfdirect_curvepoints.resize(array_size);
			for (uint32_t i = 0; i < array_size; ++i)
			{
				this->lpfdirect_curvepoints.at(i) = lpfdirect_curvepoint[i];
			}
			lpfdirect_curve = { &this->lpfdirect_curvepoints.at(0), array_size };
			emitter->pLPFDirectCurve = &lpfdirect_curve;
		}
	}*/

	void Filter(const XAUDIO2_FILTER_TYPE& type, const float& overq = 1.0f);
	void ApplyHighPassFilter(const float& overq = 1.0f);	//	ハイパスフィルター

	SoundDSPSetting GetDSPSetting() const { return dspSetting_; }

	SoundEmitter* GetEmitter() { return emitter_; }

private:

	SoundEmitter* emitter_ = {};

	SoundDSPSetting dspSetting_ = {};

	FLOAT32 absorptionRate_ = {};		//	吸収率

	FLOAT32 reflectionRate_ = {};		//	反射率

	std::vector<X3DAUDIO_DISTANCE_CURVE_POINT> volCurvepoints_;
	X3DAUDIO_DISTANCE_CURVE volCurve_ = { nullptr, 0 };

	std::vector<X3DAUDIO_DISTANCE_CURVE_POINT> lpfdirectCurvepoints_;
	X3DAUDIO_DISTANCE_CURVE lpfdirectCurve_ = { nullptr, 0 };

	//	デバッグ用
	float pitch_ = 1.0f;

};
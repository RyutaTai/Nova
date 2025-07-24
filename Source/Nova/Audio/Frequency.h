#pragma once

#include <d3d11.h>
#include <memory>
#include <thread>
#include <complex>
#include <vector>

#include "Audio.h"

using Complex = std::complex<double>;

//	フーリエ変換
class Frequency
{
public:
	Frequency() {}
	~Frequency() = default;

	void Initialize();
	void Update(const float& elapsedTime, std::shared_ptr<Audio> audioResource);
	void DrawDebug();

	void FFT(std::vector<Complex>& x);				//	フーリエ変換

	//	ハミング窓
	std::vector<float>	HammingWindow(const int& count);					
	float				HammingWindow(const int& index, const int& count);
	//	ブラックマン窓
	std::vector<float>	BlackmanWindow(const int& count);					
	float				BlackmanWindow(const int& index, const int& count);

	std::vector<float>	GetAmplitudeSpectrum() { return amplitudeSpectrum_; }
	float				GetAmplitudeSpectrum(const int& index) { return amplitudeSpectrum_.at(index); }

	const float	GetBPM()const { return bpm_; }	//	BPM取得

public:
	static constexpr int BlockCount_ = 512;		//	ハミング窓サンプル数(何分割するか)(SpectrumPS,SpectrumCirclePS.hlslのFFT_BLOCK_COUNTと合わせる)
	
private:
	static constexpr float	AudioPI_		= 3.14159265358979323846f;
	static constexpr double AudioPILong_	= 3.14159265358979323846264338328L;

	std::vector<float> amplitudeSpectrum_;			//	振幅スぺクトラム(周波数帯ごとのデシベル値)
	std::vector<float> oldAmplitudeSpectrum_;		//	前回の振幅スペクトラム
	std::vector<float> window_;
	float bpm_ = 120.0f;		//	BPMを保持

	float audioTimer_ = 0.0f;	//	再生時間

private:	//	デバッグ用変数
	DirectX::XMFLOAT2 spectrumGraphSize_ = { 0.0f,80.0f };	//	ImGuiのグラフのサイズ
	//	窓関数のタイプ
	enum class WindowType
	{
		Hamming,	//	ハミング窓
		Blackman,	//	ブラックマン窓
		Max
	};

};


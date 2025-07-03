#include "Frequency.h"

#include <algorithm>

#include "../../../External/imgui/imgui.h"
#include "../../Nova/Others/Converter.h"

void Frequency::Initialize()
{
    //  Hamming窓の生成
    window_ = HammingWindow(BlockCount_);

    oldAmplitudeSpectrum_.resize(BlockCount_, 0.0f);

}

void Frequency::Update(const float& elapsedTime, Audio* audioSource)
{
#if 1
    size_t          SPsize      = audioSource->GetAudioBytes();     //  オーディオのバッファサイズ取得
    const BYTE*     SPdata      = audioSource->GetAudioData();
    //std::vector<uint8_t> audioVector = ConvertToVector(SPdata, SPsize);
    const uint8_t*  audioVector = audioSource->GetAudioData(); 
	int             SPNowData   = static_cast<int>(audioSource->GetCurrentSample());    //  現在のサンプル
    int             SPNowBlock  = SPNowData / BlockCount_;                               //  現在のブロック計算

    //  FFT変換
    std::vector<Complex> windowedData;
    int spNowBlock = BlockCount_ * SPNowBlock;

    for (int i = 0; i < BlockCount_; ++i)
    {
        int index = i + spNowBlock;
        if (index < SPsize)  // 範囲内かチェック
        {
            windowedData.emplace_back(window_.at(i) * audioVector[index]);
        }
        else
        {
            windowedData.emplace_back(0.0f);    //  範囲外なら0を追加
        }
    }
    FFT(windowedData);

    //  振幅スペクトル
    amplitudeSpectrum_.clear();
    for (auto& w : windowedData)
    {
		amplitudeSpectrum_.emplace_back(sqrtf(static_cast<float>(w.real() * w.real() + w.imag() * w.imag())));
    }

    //  平滑化
    float blendRate = 0.9f;
    for (size_t i = 0; i < amplitudeSpectrum_.size() - 1; ++i)
    {
        if (i < oldAmplitudeSpectrum_.size())
        {
            amplitudeSpectrum_[i] = blendRate * oldAmplitudeSpectrum_[i] + ((1 - blendRate) * amplitudeSpectrum_[i]);
        }
        else
        {
            oldAmplitudeSpectrum_[i] = amplitudeSpectrum_[i];
        }
    }

    // oldAmplitudeSpectrumのサイズを更新する
    //oldAmplitudeSpectrum_ = amplitudeSpectrum_;

#else
    UINT32 SPsize = audioSource->GetAudioBytes();
    //auto& SPdata = audioSource->GetAudioData();
    auto& SPdata = ConvertToVector(audioSource->GetAudioData(), SPsize);
    int SPNowData = audioSource->GetCurrentSample();    // 現在のサンプル
    int SPNowBlock = SPNowData / BlockCount_;    // 現在のブロック計算

    // SPNowBlock が SPdata の範囲を超えないようにする
    if (SPNowBlock * BlockCount_ + BlockCount_ > SPsize)
    {
        SPNowBlock = (SPsize - BlockCount_) / BlockCount_;
    }

    // windowedDataのサイズをdataBlockSizeに設定する
    std::vector<Complex> windowedData(BlockCount_);

    for (int i = 0; i < BlockCount_; ++i)
    {
        // 範囲チェックを追加する
        if ((i + BlockCount_ * SPNowBlock) < SPsize)
        {
            windowedData[i] = hamming_[i] * SPdata[i + BlockCount_ * SPNowBlock];
        }
        else
        {
            windowedData[i] = 0; // 範囲外の場合は0を代入する
        }
    }

    FFT(windowedData);

    // 振幅スペクトル
    amplitudeSpectrum_.clear();
    for (auto& w : windowedData)
    {
        amplitudeSpectrum_.emplace_back(sqrtf(w.real() * w.real() + w.imag() * w.imag()));
    }

    // 平滑化
    for (size_t i = 0; i < amplitudeSpectrum_.size(); ++i)
    {
        if (i < oldAmplitudeSpectrum_.size())
        {
            amplitudeSpectrum_[i] = 0.97f * oldAmplitudeSpectrum_[i] + (0.03f * amplitudeSpectrum_[i]);
        }
        else
        {
            amplitudeSpectrum_[i] = amplitudeSpectrum_[i]; // もし oldAmplitudeSpectrum に対応するインデックスがない場合はそのまま
        }
    }

    // oldAmplitudeSpectrumのサイズを更新する
    oldAmplitudeSpectrum_ = amplitudeSpectrum_;

    //sec = bgm->GetCurrent_Time();

#endif
    
    audioTimer_ = audioSource->GetPlayTimer();

}

//  ハミング窓
//  http://www.densikairo.com/Development/Public/study_dsp/C1EBB4D8BFF4.html
//  https://cognicull.com/ja/qc1y1tr9
std::vector<float> Frequency::HammingWindow(const int& count)
{
    std::vector<float> hm;
    for (int i = 0; i < count; ++i)
    {
        float h;
        h = 0.54f - (0.46f * cosf((2 * AudioPI_ * i) / (count - 1)));   //  ハミング窓
        hm.emplace_back(h);
    }
    return hm;
}
float Frequency::HammingWindow(const int& index, const int& count)
{
	float h;
	h = 0.54f - (0.46f * cosf((2 * AudioPI_ * index) / (count - 1)));   //  ハミング窓
	return h;
}

//  ブラックマン窓
std::vector<float> Frequency::BlackmanWindow(const int& count)
{
    std::vector<float> hm;
    for (int i = 0; i < count; ++i)
    {
        float h;
        h = 0.42f - 0.5f * cosf(2 * AudioPI_ * i / (count - 1)) + 0.08f * cosf(4 * AudioPI_ * i / (count - 1));   //  ブラックマン窓
        hm.emplace_back(h);
    }
    return hm;
}
float Frequency::BlackmanWindow(const int& index, const int& count)
{
    float h;
    h = 0.42f - 0.5f * cosf(2 * AudioPI_ * index / (count - 1)) + 0.08f * cosf(4 * AudioPI_ * index / (count - 1));   //  ブラックマン窓
    return h;
}

//  高速フーリエ変換(FFT)
void Frequency::FFT(std::vector<Complex>& x)
{
	unsigned int N = static_cast<int>(x.size()), k = N, n;
	float thetaT = static_cast<float>(AudioPILong_ / N);

    //  DFT
    Complex phiT = Complex(cos(thetaT), -sin(thetaT)), T;
    while (k > 1)
    {
        n = k;
        k >>= 1;
        phiT = phiT * phiT;
        T = 1.0L;
        for (unsigned int l = 0; l < k; l++)
        {
            for (unsigned int a = l; a < N; a += n)
            {
                unsigned int b = a + k;
                Complex t = x[a] - x[b];
                x[a] += x[b];
                x[b] = t * T;
            }
            T *= phiT;
        }
    }
    //  Decimate
    unsigned int m = (unsigned int)log2(N);
    for (unsigned int a = 0; a < N; a++)
    {
        unsigned int b = a;
        //  Reverse bits
        b = (((b & 0xaaaaaaaa) >> 1) | ((b & 0x55555555) << 1));
        b = (((b & 0xcccccccc) >> 2) | ((b & 0x33333333) << 2));
        b = (((b & 0xf0f0f0f0) >> 4) | ((b & 0x0f0f0f0f) << 4));
        b = (((b & 0xff00ff00) >> 8) | ((b & 0x00ff00ff) << 8));
        b = ((b >> 16) | (b << 16)) >> (32 - m);
        if (b > a)
        {
            Complex t = x[a];
            x[a] = x[b];
            x[b] = t;
        }
    }
}

//  デバッグ描画
void Frequency::DrawDebug()
{  
    //  テンポ
    ImGui::DragFloat("BPM", &bpm_);
    
    //  PlotLinesを使用してスペクトラムデータを表示
    ImGui::DragFloat2("GraphSize", &spectrumGraphSize_.x, 0.1f);   //  グラフのサイズ
	ImVec2 graphSize = { spectrumGraphSize_.x, spectrumGraphSize_.y };
	ImGui::PlotLines("Amplitude Spectrum", amplitudeSpectrum_.data(), static_cast<int>(amplitudeSpectrum_.size()), 0, nullptr, FLT_MAX, FLT_MAX, graphSize);
	ImGui::PlotLines("Old Amplitude Spectrum", oldAmplitudeSpectrum_.data(), static_cast<int>(oldAmplitudeSpectrum_.size()), 0, nullptr, FLT_MAX, FLT_MAX, graphSize);
    
    std::vector<float> squaredValue;
    for (int i = 0; i < amplitudeSpectrum_.size(); ++i)
    {
        float value = amplitudeSpectrum_[i] * amplitudeSpectrum_[i] * 0.000004f;
        squaredValue.emplace_back(value);
    }
    ImGui::PlotLines("Squared Amplitude Spectrum", squaredValue.data(), static_cast<int>(squaredValue.size()), 0, nullptr, FLT_MAX, FLT_MAX, ImVec2(0, 80));
    ImGui::PlotLines("Hamming", window_.data(), static_cast<int>(window_.size()), 0, nullptr, FLT_MAX, FLT_MAX, ImVec2(0, 80));
    
    //  再生時間
    ImGui::DragFloat("PlayTime", &audioTimer_);
   
}

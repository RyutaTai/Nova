#include "Frequency.h"

#include <algorithm>

#include "../../../External/imgui/imgui.h"
#include "../../Nova/Others/Converter.h"

void Frequency::Initialize()
{
    //  Hamming窓の生成
    window_ = HammingWindow(BlockCount);

    oldAmplitudeSpectrum_.resize(BlockCount, 0.0f);

}

void Frequency::Update(const float& elapsedTime, Audio* audioSource)
{
#if 1   //変更
    size_t          SPsize      = audioSource->GetAudioBytes();     //  オーディオのバッファサイズ取得
    const BYTE*     SPdata      = audioSource->GetAudioData();
    //std::vector<uint8_t> audioVector = ConvertToVector(SPdata, SPsize);
    const uint8_t*  audioVector = audioSource->GetAudioData();
    int             SPNowData   = audioSource->GetCurrentSample();  //  現在のサンプル
    int             SPNowBlock  = SPNowData / BlockCount;           //  現在のブロック計算

    //  FFT変換
    std::vector<Complex> windowedData;
    int spNowBlock = BlockCount * SPNowBlock;

    for (int i = 0; i < BlockCount; ++i)
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
        amplitudeSpectrum_.emplace_back(sqrtf(w.real() * w.real() + w.imag() * w.imag()));
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
    int SPNowBlock = SPNowData / BlockCount;    // 現在のブロック計算

    // SPNowBlock が SPdata の範囲を超えないようにする
    if (SPNowBlock * BlockCount + BlockCount > SPsize)
    {
        SPNowBlock = (SPsize - BlockCount) / BlockCount;
    }

    // windowedDataのサイズをdataBlockSizeに設定する
    std::vector<Complex> windowedData(BlockCount);

    for (int i = 0; i < BlockCount; ++i)
    {
        // 範囲チェックを追加する
        if ((i + BlockCount * SPNowBlock) < SPsize)
        {
            windowedData[i] = hamming_[i] * SPdata[i + BlockCount * SPNowBlock];
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
	
#if 0   //  BPM取得(テンポ解析、テンポ推定)
    float samplingRate = 44100.0f;
    AnalyzeBPM(SPdata, SPsize,samplingRate);    //  BPM更新
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
        h = 0.54f - (0.46f * cosf((2 * AUDIO_PI * i) / (count - 1)));   //  ハミング窓
        hm.emplace_back(h);
    }
    return hm;
}
float Frequency::HammingWindow(const int& index, const int& count)
{
	float h;
	h = 0.54f - (0.46f * cosf((2 * AUDIO_PI * index) / (count - 1)));   //  ハミング窓
	return h;
}

//  ブラックマン窓
std::vector<float> Frequency::BlackmanWindow(const int& count)
{
    std::vector<float> hm;
    for (int i = 0; i < count; ++i)
    {
        float h;
        h = 0.42f - 0.5f * cosf(2 * AUDIO_PI * i / (count - 1)) + 0.08f * cosf(4 * AUDIO_PI * i / (count - 1));   //  ブラックマン窓
        hm.emplace_back(h);
    }
    return hm;
}
float Frequency::BlackmanWindow(const int& index, const int& count)
{
    float h;
    h = 0.42f - 0.5f * cosf(2 * AUDIO_PI * index / (count - 1)) + 0.08f * cosf(4 * AUDIO_PI * index / (count - 1));   //  ブラックマン窓
    return h;
}

//  フーリエ変換
void Frequency::FFT(std::vector<Complex>& x)
{
    unsigned int N = x.size(), k = N, n;
    float thetaT = AUDIO_PI_LONG / N;

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

//  ベクトルの平方和を計算する関数
double Frequency::Power(const double& a, const double& b)
{
    return sqrt(a * a + b * b);
}

//  ピークを見つける関数
void Frequency::FindPeak3(const double* r, const int& length, int* peakX) 
{
    std::vector<int> peaks;
    for (int i = 1; i < length - 1; i++) 
    {
        if (r[i] > r[i - 1] && r[i] > r[i + 1]) 
        {
            peaks.push_back(i);
        }
    }

    //  上位3つのピークを格納
    std::sort(peaks.begin(), peaks.end(), [&](int a, int b) { return r[a] > r[b]; });
	for (int i = 0; i < 3; i++)
    {
        peakX[i] = (i < peaks.size()) ? peaks[i] : -1;
    }
}

//  BPM解析
void Frequency::AnalyzeBPM(const uint8_t* data, const int& dataSize, const int& sampleRate) 
{
    // uint8_tからshortに変換する処理を行う
    std::vector<short> shortData(dataSize / sizeof(short));
    std::memcpy(shortData.data(), data, dataSize); // データをコピー

    const int FRAME_LEN = 512;
    int N = dataSize / sizeof(short) / 2 / FRAME_LEN;
    std::vector<double> vol(N, 0);

    //  フレームの音量計算
    double frame[FRAME_LEN];
    unsigned long i = 0;
    int j = 0, m = 0;
    while (i <= dataSize / sizeof(short) && m < N) 
    {
        frame[j++] = data[i]; //    フレームの音声データを取得
        if (j == FRAME_LEN) 
        {
            double sum = 0;
            for (int n = 0; n < FRAME_LEN; n++) 
            {
                sum += frame[n] * frame[n];
            }
            vol[m++] = sqrt(sum / FRAME_LEN);
            j = 0; //   次フレームへ
        }
        i += 2; //  サンプルサイズをスキップ
    }

    //  音量差分
    std::vector<double> diff(N, 0);
    for (int i = 1; i < N; i++)
    {
        diff[i] = max(0.0, vol[i] - vol[i - 1]); // 増加分のみ
    }

    //  テンポ解析
    std::vector<double> a(240 - 60 + 1, 0);
    std::vector<double> b(240 - 60 + 1, 0);
    std::vector<double> r(240 - 60 + 1, 0);
    const double s = double(sampleRate) / FRAME_LEN;

    for (int bpm = 60; bpm <= 240; bpm++) 
    {
        double aSum = 0, bSum = 0;
        double f = double(bpm) / 60;
        for (int n = 0; n < N; n++)
        {
			double win = HammingWindow(n, N);
            aSum += diff[n] * cos(2.0 * M_PI * f * n / s) * win;
            bSum += diff[n] * sin(2.0 * M_PI * f * n / s) * win;
        }
        a[bpm - 60] = aSum / N;
        b[bpm - 60] = bSum / N;
        r[bpm - 60] = sqrt(Power(a[bpm - 60], b[bpm - 60]));
    }

    // ピーク解析
    int peakX[3];
    FindPeak3(r.data(), 240 - 60 + 1, peakX);
    for (int idx = 0; idx < 3; idx++)
    {
        if (peakX[idx] < 0)
        {
            break;
        }
        int peakBPM = peakX[idx] + 60;
        bpm_ = peakBPM;
    }
}

//  デバッグ描画
void Frequency::DrawDebug()
{  
    // Plot imageData using ImGui
    ImGui::DragFloat("BPM", &bpm_);
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
    ImGui::DragFloat("PlayTime", &audioTimer_);
   
}

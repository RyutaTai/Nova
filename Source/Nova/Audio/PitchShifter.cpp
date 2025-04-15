#include "PitchShifter.h"

PitchShifter::PitchShifter()
{

}

//	ピッチシフト適応
void PitchShifter::ApplyPitchShift(float pitchShift, const std::vector<float>& input, std::vector<float>& output)
{
    std::vector<float> window = frequency_->HammingWindow(FRAME_SIZE);  //  ハミング窓生成

    int numFrames = (input.size() - FRAME_SIZE) / OVERLAP + 1;
    output.resize(input.size() + (numFrames - 1) * OVERLAP);

    for (int frame = 0; frame < numFrames; ++frame)
    {
        int offset = frame * OVERLAP;
        std::vector<Complex> timeDomain(FRAME_SIZE);
        std::vector<Complex> freqDomain(FRAME_SIZE);

        // フレームを取り出してウィンドウを適用
        for (int i = 0; i < FRAME_SIZE; ++i) 
        {
            timeDomain[i] = Complex(input[offset + i] * window[i], 0);
        }

        // フレームをFFTにより周波数領域へ変換
        frequency_->FFT(timeDomain);

        // ピッチシフトの適用
        int newSize = static_cast<int>(FRAME_SIZE / pitchShift);
        std::vector<Complex> shiftedFreqDomain(newSize);

        for (int i = 0; i < FRAME_SIZE / 2 + 1; ++i)
        {
            int shiftedIndex = static_cast<int>(i * pitchShift);
            if (shiftedIndex < newSize) 
            {
                shiftedFreqDomain[shiftedIndex] = timeDomain[i];
            }
        }

        // ゼロパディング
        for (int i = newSize; i < FRAME_SIZE; ++i) 
        {
            shiftedFreqDomain.emplace_back(Complex(0, 0));
        }

        // 逆FFTにより時間領域へ戻す
        frequency_->FFT(shiftedFreqDomain); // Inverse FFT (仮定：同じ関数が逆方向にも使える)

        // ウィンドウを再適用してオーバーラップ加算
        for (int i = 0; i < FRAME_SIZE; ++i) 
        {
            output[offset + i] += shiftedFreqDomain[i].real() * window[i];
        }
    }
}

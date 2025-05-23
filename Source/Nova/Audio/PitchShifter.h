#pragma once

#include "Frequency.h"

const int SAMPLE_RATE	= 44100;
const int FRAME_SIZE	= 2048;
const int OVERLAP		= FRAME_SIZE / 2;

//	ピッチシフト用クラス
class PitchShifter
{
public:
	PitchShifter();
	~PitchShifter() {}

	//	----- ピッチシフトを適応 -----
	void ApplyPitchShift(const float& pitchShift, const std::vector<float>& input, std::vector<float>& output);	//	ピッチシフト適応

private:
	Frequency* frequency_ = nullptr;

};


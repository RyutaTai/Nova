#pragma once

#include "../Nova/Audio/Midi.h"

class JudgeRhythm
{
public:
	JudgeRhythm() {}
	~JudgeRhythm() {}

	void Initialize();
	void Update();
	void DrawDebug();

	static JudgeRhythm& Instance()
	{
		static JudgeRhythm rhythm;
		return rhythm;
	}
	
	const double GetCurrentMidiTime()const { return midi_->GetCurrentTimer(); }

	//	関数を呼んだタイミングがリズムにあっているか
	bool RythmJudge();

	//	BPM
	void	SetBPM(const float& bpm){ bpm_ = bpm; }
	float	GetBPM()				{ return bpm_; }

	//	midiを見てノートオンならtrueを返す(テンポに合わせた動きをさせるために使用する)
	bool	GetRhythm();

	//	コンボ
	void		SetComboCount(const int& comboCount)	{ comboCount_ = comboCount; }
	void		AddComboCount(const int& comboCount);
	const int	GetComboCount() const					{ return comboCount_; }

private:
	float bpm_ = 140.0f;	//	楽曲のbpm

	//	midiデータ
	std::unique_ptr<Midi> midi_ = nullptr;	//	タイミング判定用midi(4つ打ち)

	//	コンボ数
	int comboCount_ = 0;

	//	デバッグ用変数
	double	debugMaxMidiTimer_ = 0.0f;
	float	debugDelta_ = 0.0f;
	float	debugClosestNoteTime_ = 0.0f;
	float	debugInputTime_ = 0.0f;

};


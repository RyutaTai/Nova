#pragma once

#include "../Nova/Audio/Midi.h"

//	リズムに関する判定を行うクラス
class JudgeRhythm
{
public:
	//	リズム判定の種類
	enum class JudgmentType
	{
		Perfect = 0,	//	パーフェクト
		Good,			//	グッド
		Miss,			//	ミス
		None,			//	判定済み
		Max
	};

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

	//	関数を呼んだタイミングがリズムにあっているかを判定する
	bool	Judge();

	//	----- BPM -----
	void	SetBPM(const float& bpm){ bpm_ = bpm; }
	float	GetBPM()				{ return bpm_; }

	//	midiを見てノートオンならtrueを返す(テンポに合わせた動きをさせるために使用する)
	bool	GetRhythm();

	//	----- コンボ -----
	void		SetComboCount(const int& comboCount)	{ comboCount_ = comboCount; }
	void		AddComboCount(const int& comboCount);
	const int	GetComboCount() const					{ return comboCount_; }

private:
	float bpm_ = 140.0f;	//	楽曲のbpm

	//	midiデータ
	std::unique_ptr<Midi> midi_ = nullptr;	//	タイミング判定用midi(4つ打ち)

	//	コンボ数
	int comboCount_ = 0;

	//	----- 各判定の範囲 -----
	float perfectRange_ = 36.0f;	//	Perfectの範囲
	float goodRange_ = 72.0f;		//	Goodの範囲

	//	デバッグ用変数
	double	debugMaxMidiTimer_ = 0.0f;
	float	debugDelta_ = 0.0f;
	float	debugClosestNoteTime_ = 0.0f;
	float	debugInputTime_ = 0.0f;
	float	debugJudgeRange_ = 0.0f;		//	判定を取った時の、中心円からの距離

};


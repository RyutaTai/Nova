#pragma once

#include "UI.h"
#include "UITempo.h"
#include "../../Game/JudgeRhythm.h"

//	”»’è•¶ŽšUI
class UIRhythmJudgment :public UI
{
public:
	UIRhythmJudgment(const JudgeRhythm::JudgmentType& judgmentType);
	~UIRhythmJudgment()override = default;

	void Initialize()override;
	void Update(const float& elapsedTime)override;
	void Render()override;
	void DrawDebug()override;

	void TextSetting(const JudgeRhythm::JudgmentType& type);

	void SetIsVisible(const bool& isVisible)override;

private:
	std::unique_ptr<Sprite> judgmentText_;	//	”»’è•¶Žš

	float displayDuration_ = 1.5f;		//	”»’è•¶Žš‚ð•`‰æ‚·‚éŽžŠÔ						
	float elapsedDisplayTime_ = 0.0f;	//	”»’è•¶Žš‚ð•\Ž¦‚µ‚ÄŒo‰ß‚µ‚½ŽžŠÔ

	//Rhythm::JudgmentType currentJudgementType_ = {};

};


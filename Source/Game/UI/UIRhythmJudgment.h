#pragma once

#include "UI.h"
#include "../../Game/Rhythm.h"

//	”»’è•¶ŽšUI
class UIRhythmJudgment :public UI
{
public:
	UIRhythmJudgment(const Rhythm::JudgmentType& judgmentType);
	~UIRhythmJudgment() {}

	void Initialize()override;
	void Update(const float& elapsedTime)override;
	void Render()override;
	void DrawDebug()override;

	void TextSetting(const Rhythm::JudgmentType& type);

	void SetIsVisible(const bool& isVisible)override;

private:
	std::unique_ptr<Sprite> judgmentText_;	//	”»’è•¶Žš

	float displayDuration_ = 1.5f;		//	”»’è•¶Žš‚ð•`‰æ‚·‚éŽžŠÔ						
	float elapsedDisplayTime_ = 0.0f;	//	”»’è•¶Žš‚ð•\Ž¦‚µ‚ÄŒo‰ß‚µ‚½ŽžŠÔ

	//Rhythm::JudgmentType currentJudgementType_ = {};

};


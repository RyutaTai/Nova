#include "UIRhythmJudgment.h"

#include "UIManager.h"
#include "../../../External/imgui/imgui.h"

UIRhythmJudgment::UIRhythmJudgment(const Rhythm::JudgmentType& judgmentType)
	:UI(UIManager::UIType::Rhythm)
{
	judgmentText_ = std::make_unique<Sprite>(L"./Resources/Image/RythmJudgment.png");

	TextSetting(judgmentType);

}

void UIRhythmJudgment::Initialize()
{
	judgmentText_->GetTransform()->SetPosition(900.0f, 1000.0f);
	isVisible_ = false;
}

void UIRhythmJudgment::Update(const float& elapsedTime)
{
	if (isVisible_) 
	{
		elapsedDisplayTime_ += elapsedTime;
		if (elapsedDisplayTime_ >= displayDuration_) 
		{
			isVisible_ = false;  //	表示フラグをオフ
			UIManager::Instance().Remove(this);
		}
	}
}

void UIRhythmJudgment::Render()
{
	judgmentText_->Render();
}

//	判定結果に応じて描画の設定をする
void UIRhythmJudgment::TextSetting(const Rhythm::JudgmentType& type)
{
	SetIsVisible(true);
	if (type == Rhythm::JudgmentType::Perfect)
	{
		judgmentText_->GetTransform()->SetPosition(900.0f, 1000.0f);
		judgmentText_->GetTransform()->SetTexPosX(0.0f);
		judgmentText_->GetTransform()->SetTexSizeX(255.0f);
		judgmentText_->GetTransform()->SetSizeX(255.0f);
		judgmentText_->GetTransform()->SetDefaultSize(552.0f, 42.0f);
	}
	else if (type == Rhythm::JudgmentType::Good)
	{
		judgmentText_->GetTransform()->SetPosition(900.0f, 1000.0f);
		judgmentText_->GetTransform()->SetTexPosX(260.0f);
		judgmentText_->GetTransform()->SetTexSizeX(165.0f);
		judgmentText_->GetTransform()->SetSizeX(165.0f);
		judgmentText_->GetTransform()->SetDefaultSize(552.0f, 42.0f);
	}
	else if (type == Rhythm::JudgmentType::Miss)
	{
		judgmentText_->GetTransform()->SetPosition(900.0f, 1000.0f);
		judgmentText_->GetTransform()->SetTexPosX(430.0f);
		judgmentText_->GetTransform()->SetTexSizeX(122.0f);
		judgmentText_->GetTransform()->SetSizeX(122.0f);
		judgmentText_->GetTransform()->SetDefaultSize(552.0f, 42.0f);
	}

}

void UIRhythmJudgment::SetIsVisible(const bool& isVisible)
{
	isVisible_ = isVisible;
	elapsedDisplayTime_ = 0.0f;
}

void UIRhythmJudgment::DrawDebug()
{
	if (ImGui::TreeNode("RhythmJudgment"))
	{
		UI::DrawDebug();
		ImGui::DragFloat("DisplayDuration", &displayDuration_);
		ImGui::DragFloat("ElapsedDIspayTime", &elapsedDisplayTime_);
		ImGui::TreePop();
	}
}

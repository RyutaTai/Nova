#include "UITempo.h"

#include "../../../External/imgui/imgui.h"
#include "../JudgeRhythm.h"
#include "../UI/UIRhythmJudgment.h"
#include "../Stage/Stage.h"
#include "../../Nova/Graphics/Vignette.h"
#include "../../Nova/Input/Input.h"
#include "../../Nova/Others/MathHelper.h"

UITempo::UITempo()
	:UI(UIManager::UIType::Tempo)
{
	//	中心円
	center_ = std::make_unique<Sprite>(L"./Resources/Image/TempoUI.png");
	center_->GetTransform()->SetTexPosX(0.0f);
	center_->GetTransform()->SetTexSizeX(100.0f);
	center_->GetTransform()->SetPivot(0.5f, 0.5f);
	center_->GetTransform()->SetPosition(962, 905);

	//	中心円の座標
	float centerPosX = center_->GetTransform()->GetPositionX();
	float centerPosY = center_->GetTransform()->GetPositionY();

	//	両サイドの半円
	for (int index = 0; index < SemicircleMax_; ++index)
	{
		//	半円生成
		semicircles_[index] = std::make_unique<Semicircle>();

		//	中心円からの距離設定して位置を決める
		//	中心円からの最大距離を半円の個数で割って1つ分の距離を算出し、等間隔に配置する
		rangePerOne_ = (semicircleRangeMax_ / SemicircleMax_) /*+ semicircleOffset_*/;
		double range = rangePerOne_ * (index + 1);
		semicircles_[index]->SetInitRange(static_cast<float>(range));
		semicircles_[index]->currentRange_ = range;

		//	左
		semicircles_[index]->left_ = std::make_unique<Sprite>(L"./Resources/Image/TempoUI.png");
		semicircles_[index]->left_->GetTransform()->SetPositionY(centerPosY);
		//semicircles_[index]->left_->GetTransform()->SetPositionX(942 - range);
		semicircles_[index]->left_->GetTransform()->SetPositionX(static_cast<float>(centerPosX - range));
		semicircles_[index]->left_->GetTransform()->SetPivot(0.5f, 0.5f);
		semicircles_[index]->left_->GetTransform()->SetTexPosX(200.0f);
		semicircles_[index]->left_->GetTransform()->SetTexSizeX(100.0f);
		semicircles_[index]->left_->GetTransform()->SetDefaultSize(100.0f, 100.0f);

		//	右
		semicircles_[index]->right_ = std::make_unique<Sprite>(L"./Resources/Image/TempoUI.png");
		semicircles_[index]->right_->GetTransform()->SetPositionY(centerPosY);
		//semicircles_[index]->right_->GetTransform()->SetPositionX(982 + range);
		semicircles_[index]->right_->GetTransform()->SetPositionX(static_cast<float>(centerPosX + range));
		semicircles_[index]->right_->GetTransform()->SetPivot(0.5f, 0.5f);
		semicircles_[index]->right_->GetTransform()->SetTexPosX(300.0f);
		semicircles_[index]->right_->GetTransform()->SetTexSizeX(100.0f);
		semicircles_[index]->right_->GetTransform()->SetDefaultSize(100.0f, 100.0f);

		//	判定済みフラグ初期化
		semicircles_[index]->isJudged_ = false;

	}

	//	表示フラグをtrueにしておく。ビューボタンで切り替えできる
	SetIsVisible(true);

}

//	初期化処理
void UITempo::Initialize()
{
	centerCircleAnimFlag_ = false;
	centerAnimTime_ = 0;
}

//	更新処理
void UITempo::Update(const float& elapsedTime)
{
	UpdateDrawFlag();
	UpdatePosition(elapsedTime);
	UpdateScale(elapsedTime);
	UpdateCenterCircleAnimation();
}

//	描画フラグ切り替え処理
void UITempo::UpdateDrawFlag()
{
	bool isVisible = GetIsVisible();
	GamePad& gamePad = Input::Instance().GetGamePad();
	if (gamePad.GetButtonDown() & GamePad::BTN_BACK)	//	ビューボタンを押したら表示フラグを反転
	{
		SetIsVisible(!isVisible);
	}
}

//	UIの位置更新処理
void UITempo::UpdatePosition(const float& elapsedTime)
{
	double centerPosX = center_->GetTransform()->GetPositionX();	//	中心円のX座標
	
	double totalRange = 0.0f;
	for (int index = 0; index < SemicircleMax_; ++index)
	{
		//	range更新
		semicircles_[index]->currentRange_ -= (rangePerOne_ / quarterNoteDuration_) * elapsedTime;

		//	中心円と重なったら最大距離にリセット
		if (semicircles_[index]->currentRange_ <= semicircleRangeMin_)
		{
			semicircles_[index]->currentRange_ = semicircleRangeMax_;

			//	アニメーションフラグ
			centerCircleAnimFlag_ = true;

			//	判定済みフラグをリセット
			if (semicircles_[index]->isJudged_)semicircles_[index]->isJudged_ = false;

			//	ヴィネット範囲を最大値に変更
			Vignette::Instance().SetLerpFlag(true);

		}

		//	rangeを元に位置を更新
		double range = semicircles_[index]->currentRange_;
		semicircles_[index]->left_->GetTransform()->SetPositionX(static_cast<float>(centerPosX - range));
		semicircles_[index]->right_->GetTransform()->SetPositionX(static_cast<float>(centerPosX + range));

		//	合計距離更新
		totalRange += range;

	}
	totalRange_ = totalRange;
}

//	UIのスケール更新処理
void UITempo::UpdateScale(const float& elapsedTime)
{
	for (int index = 0; index < SemicircleMax_; ++index)
	{
		//	半円更新
		float range = static_cast<float>(semicircles_[index]->currentRange_);
		float normalizeRange = static_cast<float>((range - semicircleRangeMin_) / (semicircleRangeMax_ - semicircleRangeMin_));										//	rangeを正規化
		float scaleFactor = semicircleScaleMin_ + normalizeRange * (semicircleScaleMax_ - semicircleScaleMin_);	//	スケール算出
		semicircles_[index]->left_->GetTransform()->SetScaleFactor(scaleFactor);
		semicircles_[index]->right_->GetTransform()->SetScaleFactor(scaleFactor);
	}
}

//	中心円のアニメーション更新
void UITempo::UpdateCenterCircleAnimation()
{
	//	中心円のアニメーション更新フラグがfalseなら処理しない
	if (centerCircleAnimFlag_ == false)return;

	center_->GetTransform()->SetTexPosX(100.0f);

	if (centerAnimTime_ > animChangeThreshold_)
	{
		centerCircleAnimFlag_ = false;
		center_->GetTransform()->SetTexPosX(0.0f);
		centerAnimTime_ = 0;

	}
	centerAnimTime_++;

}

//	中心円に一番近い半円の番号を見つける
int UITempo::FindNearSemicircleIndex()
{
	double nearRange = DBL_MAX;			//	中心円に一番近い半円の最短距離
	int nearSemicircleIndex = INT_MAX;	//	中心円に一番近い半円の番号

	for (int i = 0; i < SemicircleMax_; ++i)
	{
		//	前回より中心円に近い半円があれば、最短距離と番号を更新する
		if (nearRange > semicircles_[i]->currentRange_)
		{
			nearSemicircleIndex = i;
			nearRange = semicircles_[i]->currentRange_;
		}
	}
	return nearSemicircleIndex;
}

//	描画処理
void UITempo::Render()
{
	center_->Render();
	for (int index = 0; index < SemicircleMax_; ++index)
	{
		//	判定済みなら描画しない(消えたように見せる)
		if (semicircles_[index]->isJudged_)continue;

		semicircles_[index]->left_->Render();
		semicircles_[index]->right_->Render();
	}
}

void UITempo::DrawDebug()
{
	if (ImGui::TreeNode("Tempo"))
	{
		UI::DrawDebug();
		float bpm = JudgeRhythm::Instance().GetBPM();
		ImGui::DragFloat("BPM", &bpm, 0.1f);
		ImGui::DragFloat("QuarterNoteDuration", &quarterNoteDuration_);	

		//	描画フラグ
		ImGui::Checkbox("IsVisible", &isVisible_);
		
		ImGui::Text("----- Center -----");
		ImGui::DragInt("AnimChangeThreshold_", &animChangeThreshold_);
		center_->DrawDebug();

		ImGui::Text("----- Range -----");
		float rangePerOne = static_cast<float>(rangePerOne_);
		ImGui::DragFloat("RangePerOne", &rangePerOne);
		ImGui::DragFloat("TotalRange", &totalRange_);
		float semicircleRangeMax = static_cast<float>(semicircleRangeMax_);
		float semicircleRangeMin = static_cast<float>(semicircleRangeMin_);
		ImGui::DragFloat("RangeMax", &semicircleRangeMax);
		ImGui::DragFloat("RangeMin", &semicircleRangeMin);

		ImGui::Text("----- Semicircle -----");
		for (int i = 0; i < UITempo::SemicircleMax_; ++i)
		{
			std::string name = "Semi" + std::to_string(i);
			if (ImGui::TreeNode(name.c_str()))
			{
				if (ImGui::TreeNode("Left"))
				{
					ImGui::PushID(static_cast<int>(Side::Left));
					if (ImGui::TreeNode("SpriteTransform"))
					{
						semicircles_[i]->left_->DrawDebug();
						ImGui::TreePop();
					}
					ImGui::PopID();
					ImGui::TreePop();
				}

				if (ImGui::TreeNode("Right"))
				{
					ImGui::PushID(static_cast<int>(Side::Right));
					if (ImGui::TreeNode("SpriteTransform"))
					{
						semicircles_[i]->right_->DrawDebug();
						ImGui::TreePop();
					}
					ImGui::PopID();
					ImGui::TreePop();
				}
				
				float currentRange = static_cast<float>(semicircles_[i]->currentRange_);
				ImGui::DragFloat("Range", &currentRange);

				//	スケール確認用
				static float scaleFactor = 1.0f;
				ImGui::DragFloat("Scale", &scaleFactor, 0.01f);
				semicircles_[i]->left_->GetTransform()->Scaling(scaleFactor);

				ImGui::TreePop();
			}

		}
		ImGui::TreePop();
	}
}
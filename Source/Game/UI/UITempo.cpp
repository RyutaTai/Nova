#include "UITempo.h"

#include "../../../External/imgui/imgui.h"
#include "../../Nova/Input/Input.h"
#include "../Rhythm.h"

UITempo::UITempo()
	:UI(UIManager::UIType::Tempo)
{
	//	真ん中の円
	center_ = std::make_unique<Sprite>(L"./Resources/Image/TempoUI.png");
	center_->GetTransform()->CutOutX(-300.0f);
	center_->GetTransform()->SetPivot(0.5f, 0.5f);
	center_->GetTransform()->SetTexPosX(0.0f);
	center_->GetTransform()->SetPosition(962, 905);

	//	中心円のY座標
	float centerPosY = center_->GetTransform()->GetPositionY();

	//	両サイドの半円
	for (int index = 0; index < SemicircleMax; ++index)
	{
		//	半円生成
		semicircles_[index] = std::make_unique<Semicircle>();

		//	range_設定
		float RangePerOne = semicircleRangeMax_ / SemicircleMax;	//	一つ当たりのrange_
		float range = RangePerOne * index;
		semicircles_[index]->range_ = range;

		//	左
		semicircles_[index]->left_ = std::make_unique<Sprite>(L"./Resources/Image/TempoUI.png");
		semicircles_[index]->left_->GetTransform()->SetPositionY(centerPosY);
		semicircles_[index]->left_->GetTransform()->SetPositionX(942 - range);
		semicircles_[index]->left_->GetTransform()->SetPivot(0.5f, 0.5f);
		semicircles_[index]->left_->GetTransform()->SetTexPosX(200.0f);
		semicircles_[index]->left_->GetTransform()->SetTexSizeX(100.0f);
		semicircles_[index]->left_->GetTransform()->SetSizeX(100.0f);
		semicircles_[index]->left_->GetTransform()->SetDefaultSize(100.0f, 100.0f);

		//	右
		semicircles_[index]->right_ = std::make_unique<Sprite>(L"./Resources/Image/TempoUI.png");
		semicircles_[index]->right_->GetTransform()->SetPositionY(centerPosY);
		semicircles_[index]->right_->GetTransform()->SetPositionX(982 + range);
		semicircles_[index]->right_->GetTransform()->SetPivot(0.5f, 0.5f);
		semicircles_[index]->right_->GetTransform()->SetTexPosX(300.0f);
		semicircles_[index]->right_->GetTransform()->SetTexSizeX(100.0f);
		semicircles_[index]->right_->GetTransform()->SetSizeX(100.0f);
		semicircles_[index]->right_->GetTransform()->SetDefaultSize(100.0f, 100.0f);
	}

	//	表示フラグをtrueにしておく。ビューボタンで切り替えできる
	SetIsVisible(true);
	//SetDrawFlag(false);

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
	UpdateMoveFactor();
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
	float centerPosX = center_->GetTransform()->GetPositionX();	//	中心円のX座標
	
	for (int index = 0; index < SemicircleMax; ++index)
	{
		//	range更新
		semicircles_[index]->range_ -= moveSpeed_ * moveFactor_ * elapsedTime;
		if (semicircles_[index]->range_ <= semicircleRangeMin_)	//	中心円と重なったら最大距離にリセット
		{
			semicircles_[index]->range_ = semicircleRangeMax_;
			centerCircleAnimFlag_ = true;
		}

		float range = semicircles_[index]->range_;

		//	位置更新
		semicircles_[index]->left_->GetTransform()->SetPositionX(centerPosX - range);
		semicircles_[index]->right_->GetTransform()->SetPositionX(centerPosX + range);
	}

}

//	UIのスケール更新処理
void UITempo::UpdateScale(const float& elapsedTime)
{
	for (int index = 0; index < SemicircleMax; ++index)
	{
		//	半円更新
		float range = semicircles_[index]->range_;
		float normalizeRange = (range - semicircleRangeMin_) / (semicircleRangeMax_ - semicircleRangeMin_);										//	rangeを正規化
		float scaleFactor = semicircleScaleMin_ + normalizeRange * (semicircleScaleMax_ - semicircleScaleMin_);	//	スケール算出
		semicircles_[index]->left_->GetTransform()->SetScaleFactor(scaleFactor);
		semicircles_[index]->right_->GetTransform()->SetScaleFactor(scaleFactor);
	}
}

//	BPMに合わせたMoveSpeedの更新
void UITempo::UpdateMoveFactor()
{
	//	bpmに合わせた速度の設定
	moveFactor_ = (Rhythm::Instance().GetBPM() / 120.0f);

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

//	描画処理
void UITempo::Render()
{
	center_->Render();
	for (int index = 0; index < SemicircleMax; ++index)
	{
		semicircles_[index]->left_->Render();
		semicircles_[index]->right_->Render();
	}
}

void UITempo::DrawDebug()
{
	if (ImGui::TreeNode("Tempo"))
	{
		UI::DrawDebug();
		float bpm = Rhythm::Instance().GetBPM();
		ImGui::DragFloat("BPM", &bpm, 0.1f);
		
		//	描画フラグ
		ImGui::Checkbox("IsVisible", &isVisible_);
		
		ImGui::Text("Center");								//	中心の円
		ImGui::DragInt("animChangeThreshold_", &animChangeThreshold_);
		ImGui::DragFloat("RangeMax", &semicircleRangeMax_);
		ImGui::DragFloat("RangeMin", &semicircleRangeMin_);
		center_->DrawDebug();

		ImGui::DragFloat("MoveSpeed", &moveSpeed_, 0.1f);	//	半円が移動する速さ
		ImGui::DragFloat("MoveFactor", &moveFactor_, 0.1f);	//	半円が移動する速さの倍率
		if (ImGui::TreeNode("Semi0"))
		{
			ImGui::PushID(static_cast<int>(Side::Left));
			semicircles_[0]->left_->DrawDebug();
			ImGui::PopID();

			ImGui::PushID(static_cast<int>(Side::Right));
			semicircles_[0]->right_->DrawDebug();
			ImGui::PopID();

			ImGui::DragFloat("Range", &semicircles_[0]->range_);

			//	スケール確認用
			static float scaleFactor = 1.0f;
			ImGui::DragFloat("Scale", &scaleFactor, 0.01f);
			semicircles_[0]->left_->GetTransform()->Scaling(scaleFactor);

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Semi1"))
		{
			ImGui::PushID(static_cast<int>(Side::Left));
			semicircles_[1]->left_->DrawDebug();
			ImGui::PopID();

			ImGui::PushID(static_cast<int>(Side::Right));
			semicircles_[1]->right_->DrawDebug();
			ImGui::PopID();

			ImGui::DragFloat("Range", &semicircles_[1]->range_);
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Semi2"))
		{
			ImGui::PushID(static_cast<int>(Side::Left));
			semicircles_[2]->left_->DrawDebug();
			ImGui::PopID();

			ImGui::PushID(static_cast<int>(Side::Right));
			semicircles_[2]->right_->DrawDebug();
			ImGui::PopID();

			ImGui::DragFloat("Range", &semicircles_[2]->range_);
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Semi3"))
		{
			ImGui::PushID(static_cast<int>(Side::Left));
			semicircles_[3]->left_->DrawDebug();
			ImGui::PopID();

			ImGui::PushID(static_cast<int>(Side::Right));
			semicircles_[3]->right_->DrawDebug();
			ImGui::PopID();

			ImGui::DragFloat("Range", &semicircles_[3]->range_);
			ImGui::TreePop();
		}
		ImGui::TreePop();
	}
}
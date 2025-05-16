#include "UIRank.h"

#include "../JudgeRhythm.h"
#include "../../Nova/Graphics/Graphics.h"
#include "../../../External/imgui/imgui.h"

UIRank::UIRank()
	:UI(UIManager::UIType::Rank)
{
	Sprite::InitInfo initInfo = {};
	initInfo.psFilename_ = "./Resources/Shader/UIRankPs.cso";
	rankTextBack_ = std::make_unique<Sprite>(L"./Resources/Image/ComboRankBack.png");
	rankText_ = std::make_unique<Sprite>(L"./Resources/Image/ComboRank.png",initInfo);

	//	次のランクへのしきい値
	pointsToNextRank_[0] = 15.0f;	//	B
	pointsToNextRank_[1] = 30.0f;	//	A
	pointsToNextRank_[2] = 45.0f;	//	S

}

void UIRank::Initialize()
{
	//	ランクポイント
	totalRankPoint_ = 0.0f;
	currentRankPoint_ = 0.0f;

	//	ランク文字
	rankTextBack_->GetTransform()->SetTexSizeX(RankTextSize_);
	rankTextBack_->GetTransform()->SetPivot(0.5f, 0.5f);
	rankTextBack_->GetTransform()->SetTexPosX(0.0f);
	rankTextBack_->GetTransform()->SetPosition(1580.0f, 330.0f);
	//	ランク文字の背景
	rankText_->GetTransform()->SetTexSizeX(RankTextSize_);
	rankText_->GetTransform()->SetPivot(0.5f, 0.5f);
	rankText_->GetTransform()->SetTexPosX(0.0f);
	rankText_->GetTransform()->SetPosition(1580.0f, 330.0f);

	//	現在のランク
	currentRankInfo_.index_ = static_cast<int>(RankType::C);
	currentRankInfo_.pointToNextRank_ = pointsToNextRank_[static_cast<int>(RankType::C)];
	//currentRank_ = RankType::C;

	D3D11_BUFFER_DESC desc = {};
	desc.ByteWidth = 64;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;
	HRESULT hr = Graphics::Instance().GetDevice()->CreateBuffer(&desc, nullptr, constantBuffer_.ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

}

void UIRank::Update(const float& elapsedTime)
{
	UpdateVisibleThreshold(elapsedTime);
}

//	ランクUIの切り取り幅をランクポイントに応じて更新する
void UIRank::UpdateVisibleThreshold(const float& elapsedTime)
{
	//	現在のランク階級
	int rankGrade = currentRankInfo_.index_;
	if (rankGrade >= RankTypeMax_ - 2)
		rankGrade = RankTypeMax_ - 2;
	//	threshold = (threshold最大値 - threshold最小値) * (現在のランクポイント / 次のランクに必要なポイント) + threshold最小値
	visibleThreshold_ = (VisibleThresholdMax_ - VisibleThresholdMin_) 
		* (currentRankPoint_ / pointsToNextRank_[rankGrade]) + VisibleThresholdMin_;

	//	次のランクへのポイントが貯まっていて、ランクがSランクより低ければランクアップ処理
	if (visibleThreshold_ >= VisibleThresholdMax_ && currentRankInfo_.index_ <= 2)
	{
		//	画像の切り取り開始位置を更新
		rankText_->GetTransform()->AddTexPosX(RankTextSize_);
		rankTextBack_->GetTransform()->AddTexPosX(RankTextSize_);

		visibleThreshold_ = VisibleThresholdMin_;
		
		//	現在のランク情報更新
		currentRankInfo_.index_++;
		if (currentRankInfo_.index_ < RankTypeMax_ - 2)
			currentRankInfo_.pointToNextRank_ = pointsToNextRank_[currentRankInfo_.index_];
		currentRankPoint_ = 0.0f;
	}

}

//	ランクポイント加算
void UIRank::AddRankPoint(const float& addRankPoint)
{
	totalRankPoint_ += addRankPoint;
	currentRankPoint_ += addRankPoint;
}

void UIRank::Render()
{
	rankTextBack_->Render();
	Graphics::Instance().GetDeviceContext()->UpdateSubresource(constantBuffer_.Get(), 0, 0, &visibleThreshold_, 0, 0);
	rankText_->Render(0,constantBuffer_.GetAddressOf());
}

void UIRank::DrawDebug()
{
	if (ImGui::TreeNode("Rank"))
	{
		ImGui::Text("----- RankPoint -----");
		ImGui::DragFloat("TotalRankPoint", &totalRankPoint_);
		ImGui::DragFloat("CurrentRankPoint", &currentRankPoint_);

		ImGui::DragFloat("Threshold", &visibleThreshold_, 0.01f, 0.0f, 1.0f);
		ImGui::DragInt("CurrrentRankIndex", &currentRankInfo_.index_);
		ImGui::DragFloat("PointsToNextRank", &currentRankInfo_.pointToNextRank_);
		if (ImGui::TreeNode("RankText"))
		{
			rankText_->DrawDebug();
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("RankTextBack"))
		{
			rankTextBack_->DrawDebug();
			ImGui::TreePop();
		}
		ImGui::TreePop();
	}
}
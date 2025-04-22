#pragma once

#include "UI.h"
#include "UIManager.h"

//	ランクUI
class UIRank :public UI
{
private:
	//	ランクの種類
	enum class RankType
	{
		C,B,A,S
	};

	//	ランク情報
	struct RankInfo
	{
		int		index_ = 0;					//	現在のランク
		float	pointToNextRank_ = 0.0f;	//	次のランクへのポイント
	};

public:
	UIRank();
	~UIRank() {}

	void Initialize()override;
	void Update(const float& elapsedTime)override;
	void Render()override;
	void DrawDebug()override;

	void UpdateVisibleThreshold(const float& elapsedTime);

	//	ランクポイント
	void AddRankPoint(const float& addRankPoint);
	void SetTotalRankPoint(const float& rankPoint) { totalRankPoint_ = rankPoint; }
	void SetCurrentRankPoint(const float& rankPoint) { currentRankPoint_ = rankPoint; }
	const float GetTotalRankPoint() { return totalRankPoint_; }
	const float GetCurrentRankPoint() { return currentRankPoint_; }

	void SetVisibleThreshold(const float& threshold) { visibleThreshold_ = threshold; }
	float GetVisibleThreshold() { return visibleThreshold_; }

private:
	std::unique_ptr<Sprite> rankText_;			//	ランクの文字(ランクポイントに応じて切り取り幅を変化させる)
	std::unique_ptr<Sprite> rankTextBack_;		//	ランクの文字の背景
	
	//	ランク文字のY方向の切り取り幅を変化させる処理に使用する変数
	static constexpr float VisibleThresholdMax_ = 0.8f;
	static constexpr float VisibleThresholdMin_ = 0.2f;
	float visibleThreshold_ = 0.0f;	//	0.2f～0.8fまで
	Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer_;

	static const int	RankTypeMax_ = 4;							//	ランクの種類
	static constexpr float RankTextSize_ = 512.0f;					//	ランク1つ分のサイズ(512*512)
	//RankType			currentRank_ = RankType::C;					//	現在のランク
	RankInfo			currentRankInfo_ = {};						//	現在のランク情報
	float				pointsToNextRank_[RankTypeMax_ - 1] = {};	//	それぞれのランクへの閾値
	float				totalRankPoint_ = 0.0f;						//	累計ランクポイント
	float				currentRankPoint_ = 0.0f;					///	現在のランクでのランクポイント

};


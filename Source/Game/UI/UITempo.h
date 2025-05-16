#pragma once

#include "UI.h"
#include "UIManager.h"

//	テンポガイドUI	真ん中の円1つ、サイドの半円4つずつ
class UITempo : public UI
{
private:
	//	半円の構造体(左右のペアで管理)
	struct Semicircle
	{
		std::unique_ptr<Sprite> left_;		//	左側の半円
		std::unique_ptr<Sprite> right_;		//	右側の半円
		float	range_ = 0.0f;				//	中心円からの距離
		bool	isJudged_ = false;			//	判定済みかどうか

	public:
		Sprite* GetLeft()	{ return left_.get(); }			//	左側の半円取得
		Sprite* GetRight()	{ return right_.get(); }		//	右側の半円取得
		
		void SetRage(const float& range) { range_ = range; }
		const float GetRange()const { return range_; }		//	中心円からの距離取得
		
		void SetIsJudged(const bool& isJudged) { isJudged_ = isJudged; }
		const bool	IsJudged()const { return isJudged_; }	//	判定済みフラグ取得

	};

	//	左右の識別(ImGui用)
	enum class Side
	{
		Left = 0,
		Right,
		Max
	};

public:
	UITempo();
	~UITempo() {}

	void Initialize()override;
	void Update(const float& elapsedTime)override;
	void Render()override;
	void DrawDebug()override;

	Sprite*		GetCenterCircle()				{ return center_.get(); }				//	中心円取得
	Semicircle* GetSemicircle(const int& index) { return semicircles_[index].get(); }	//	半円取得
	const float GetTotalRange()const			{ return totalRange_; }					//	中心円からのそれぞれの半円の合計

	int FindNearSemicircleIndex();					//	中心円に一番近い半円の番号を見つける

private:
	void UpdateDrawFlag();							//	描画フラグ切り替え処理
	void UpdateCenterCircleAnimation();				//	中心円のアニメーション更新処理
	void UpdatePosition(const float& elapsedTime);	//	UIの位置更新処理
	void UpdateScale(const float& elapsedTime);		//	UIのスケール更新処理

private:
	static constexpr int		SemicircleMax_ = 4;				//	半円の数
	std::unique_ptr<Sprite>		center_;						//	テンポガイドの中心
	std::unique_ptr<Semicircle> semicircles_[SemicircleMax_];	//	半円の組
	
	float quarterNoteDuration_ = 0.429f;	//	BPM140のときの、4分音符1つ分の長さ

	//	中心円からの距離
	float	rangePerOne_ = 1.0f;				//	半円1つ当たりの距離 ( 最大距離/個数 に設定し、等間隔に配置する)
	float	totalRange_ = 0.0f;					//	それぞれの距離の合計
	float	semicircleRangeMax_ = 576.0f;		//	rangeの最大値
	float	semicircleRangeMin_ = -0.5f;		//	rangeの最小値。これを下回ったら位置リセット
	
	//	スケール
	float	centerScaleMax_ = 1.0f;				//	中心円のスケール最大値
	float	centerScaleMin_ = 0.75f;			//	中心円のスケール最小値
	float	semicircleScaleMax_ = 1.5f;			//	半円のスケール最大値
	float	semicircleScaleMin_ = 1.0f;			//	半円のスケール最小値

	//	アニメーション
	bool	centerCircleAnimFlag_ = false;		//	中心円のアニメーション更新フラグ
	int		animChangeThreshold_ = 9;			//	何フレームでアニメーションを遷移するか
	int		centerAnimTime_ = 0;				//	中心円のアニメーション時間カウント

};


#pragma once

#include "UI.h"

//	テンポガイドUI	真ん中の円一つ、サイドの半円4つずつ
class UITempo : public UI
{
public:
	UITempo();
	~UITempo() {}

	void Initialize()override;
	void Update(const float& elapsedTime)override;
	void Render()override;
	void DrawDebug()override;

	void SetIsVisible(const bool& isVisible)override { isVisible_ = isVisible; }

	void UpdateDrawFlag();
	void UpdateCenterCircleAnimation();
	void UpdatePosition(const float& elapsedTime);
	void UpdateScale(const float& elapsedTime);
	void UpdateMoveFactor();

private:
	//	半円の構造体
	struct Semicircle
	{
		std::unique_ptr<Sprite> left_;		//	左側の半円
		std::unique_ptr<Sprite> right_;		//	右側の半円
		float range_ = 0.0f;				//	中心円からの距離
	};
	
	//	左右の識別(ImGui用)
	enum class Side
	{
		Left = 0,
		Right,
		Max
	};

private:
	static constexpr int		SemicircleMax = 4;				//	半円の数
	std::unique_ptr<Sprite>		center_;						//	テンポガイドの中心
	std::unique_ptr<Semicircle> semicircles_[SemicircleMax];	//	半円の組
	
	float	semicircleRangeMax_ = 576.0f;		//	中心円からの距離の最大値
	float	semicircleRangeMin_ = -0.5f;		//	rangeの最小値。これを下回ったら位置リセット
	float	centerScaleMax_ = 1.0f;				//	真ん中の円のスケール最大値
	float	centerScaleMin_ = 0.75f;			//	真ん中の円のスケール最小値
	float	semicircleScaleMax_ = 1.5f;			//	半円のスケール最大値
	float	semicircleScaleMin_ = 1.0f;			//	半円のスケール最小値

	float	moveSpeed_ = 290.0f;				//	移動する速さ
	float	moveFactor_ = 1.0f;					//	BPM120を基準とする移動する速さの倍率
	bool	centerCircleAnimFlag_ = false;		//	中心円のアニメーション更新フラグ
	int		animChangeThreshold_ = 9;			//	何フレームでアニメーションを遷移するか
	int		centerAnimTime_ = 0;				//	中心円のアニメーション時間カウント


};


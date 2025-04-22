#pragma once

#include "UI.h"
#include "UIManager.h"

//	テンポガイドUI	真ん中の円1つ、サイドの半円4つずつ
class UITempo : public UI
{
public:
	//	判定の種類
	enum class JudgmentType
	{
		Perfect = 0,
		Good,
		Miss,			//	ミス
		None,			//	判定済み
		Max
	};

public:
	UITempo();
	~UITempo() {}

	void Initialize()override;
	void Update(const float& elapsedTime)override;
	void Render()override;
	void DrawDebug()override;

	bool JudgeRythm();	//	入力タイミングがリズムにあっているか判定

private:
	void UpdateDrawFlag();
	void UpdateCenterCircleAnimation();
	void UpdatePosition(const float& elapsedTime);
	void UpdateScale(const float& elapsedTime);
	void UpdateMoveFactor();

private:
	int FindNearSemicircleIndex();	//	中心円に一番近い半円の番号を見つける

private:
	//	半円の構造体
	struct Semicircle
	{
		std::unique_ptr<Sprite> left_;		//	左側の半円
		std::unique_ptr<Sprite> right_;		//	右側の半円
		float	range_ = 0.0f;				//	中心円からの距離
		bool	isJudged_ = false;			//	判定済みかどうか
	};
	
	//	左右の識別(ImGui用)
	enum class Side
	{
		Left = 0,
		Right,
		Max
	};

private:
	static constexpr int		SemicircleMax_ = 4;				//	半円の数
	std::unique_ptr<Sprite>		center_;						//	テンポガイドの中心
	std::unique_ptr<Semicircle> semicircles_[SemicircleMax_];	//	半円の組
	
	float quarterNoteDuration_ = 0.429f;	//	BPM140のときの、4分音符1つ分の長さ

	//	中心円からの距離
	float	rangePerOne_ = 1.0f;				//	半円1つ当たりの距離 ( 最大距離/個数 に設定し、等間隔に配置する)
	float	totalRange_ = 0.0f;					//	それぞれの距離の合計
	//float	semicircleRangeMax_ = 432.0f;		//	rangeの最大値
	float	semicircleRangeMax_ = 576.0f;		//	rangeの最大値
	float	semicircleRangeMin_ = -0.5f;		//	rangeの最小値。これを下回ったら位置リセット
	
	//	スケール
	float	centerScaleMax_ = 1.0f;				//	真ん中の円のスケール最大値
	float	centerScaleMin_ = 0.75f;			//	真ん中の円のスケール最小値
	float	semicircleScaleMax_ = 1.5f;			//	半円のスケール最大値
	float	semicircleScaleMin_ = 1.0f;			//	半円のスケール最小値

	//	移動
	float	moveSpeed_ = 290.0f;				//	移動する速さ
	float	moveFactor_ = 1.0f;					//	BPM120を基準とする移動する速さの倍率
	
	//	アニメーション
	bool	centerCircleAnimFlag_ = false;		//	中心円のアニメーション更新フラグ
	int		animChangeThreshold_ = 9;			//	何フレームでアニメーションを遷移するか
	int		centerAnimTime_ = 0;				//	中心円のアニメーション時間カウント

	//	各判定の範囲
	float perfectRange_ = 36.0f;	//	Perfectの範囲
	float goodRange_ = 72.0f;		//	Goodの範囲

};


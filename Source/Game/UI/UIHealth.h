#pragma once

#include "UI.h"

class UIHealth : public UI
{
public:
	UIHealth();
	~UIHealth(){}

	void Initialize()override;
	void Update(const float& elapsedTime)override;
	void Render()override;
	void DrawDebug()override;

	void SetIsVisible(const bool& isVisible)override { isVisible_ = isVisible; }

	void UpdateHpGauge(const float& elpasedTime);		//	HPゲージ更新
	void UpdateHpGaugeBack(const float& elpasedTime);	//	後ろのHPゲージ更新
	void CheckDamage();									//	ダメージを食らったか確認する


private:
	std::unique_ptr<Sprite> hpFrame_;
	std::unique_ptr<Sprite> hpGauge_;
	std::unique_ptr<Sprite> hpGaugeBack_;

	const float GAUGE_SIZE_X = 440.0f;		//	HPゲージの画像サイズX

	float oldHealth_ = 0.0f;				//	前回のHP
	bool isUpdateHpGaugeBack_ = false;		//	後ろのゲージ更新フラグ
	float startValue_ = 0.0f;				//	補完開始
	float endValue_ = 0.0f;					//	補完終了
	float gaugeLerpTimer_ = 0.0f;			//	補完にかける時間
	float gaugeLerpSpeed_ = 1.0f;			//	補完する速さ

};


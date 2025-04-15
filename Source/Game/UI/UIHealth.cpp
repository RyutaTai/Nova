#include "UIHealth.h"

#include "../Player.h"
#include "../Rhythm.h"
#include "../../Nova/Others/MathHelper.h"

UIHealth::UIHealth()
	:UI(UIManager::UIType::Health)
{
	hpGauge_ = std::make_unique<Sprite>(L"./Resources/Image/HpGauge.png");
	hpGaugeBack_ = std::make_unique<Sprite>(L"./Resources/Image/HpGaugeBack.png");
	hpFrame_ = std::make_unique<Sprite>(L"./Resources/Image/HpFrame.png");

	hpGauge_->GetTransform()->SetPosition(80, 40);
	hpGaugeBack_->GetTransform()->SetPosition(80, 40);
	hpFrame_->GetTransform()->SetPosition(0, 0);
}

void UIHealth::Initialize()
{

}

void UIHealth::Update(const float& elapsedTime)
{
	CheckDamage();
	UpdateHpGauge(elapsedTime);			//	手前のHPバー更新
	UpdateHpGaugeBack(elapsedTime);		//	後ろのHPバー更新

}

//	HPゲージ更新
void UIHealth::UpdateHpGauge(const float& elapsedTime)
{
	const float maxHealth = Player::Instance().GetMaxHp();
	const float currentHealth = Player::Instance().GetHp();
	const float health = currentHealth / maxHealth;	//	0～1にする
	hpGauge_->GetTransform()->SetSizeX(GAUGE_SIZE_X * health);

}

//	後ろのHPゲージ更新
void UIHealth::UpdateHpGaugeBack(const float& elapsedTime)
{
	if (isUpdateHpGaugeBack_ == false)return;

	gaugeLerpTimer_ += gaugeLerpSpeed_ * elapsedTime;
	gaugeLerpTimer_ = min(gaugeLerpTimer_, 1.0f);

	const float sizeX = XMFloatLerp(startValue_, endValue_, gaugeLerpTimer_);
	hpGaugeBack_->GetTransform()->SetSizeX(sizeX);

	if (gaugeLerpTimer_ == 1.0f)isUpdateHpGaugeBack_ = false;
}

//	ダメージを食らったか確認する
void UIHealth::CheckDamage()
{
	const float oldHealth = oldHealth_;
	const float currentHealth = Player::Instance().GetHp();
	const float maxHealth = Player::Instance().GetMaxHp();
	oldHealth_ = currentHealth;

	if (currentHealth >= oldHealth) return;
	
	isUpdateHpGaugeBack_ = true;

	gaugeLerpTimer_ = 0.0f;

	startValue_ = oldHealth / maxHealth * GAUGE_SIZE_X;
	endValue_ = currentHealth / maxHealth * GAUGE_SIZE_X;
}

void UIHealth::Render()
{
	hpGaugeBack_->Render();
	hpGauge_->Render();
	hpFrame_->Render();
}

void UIHealth::DrawDebug()
{
	if (ImGui::TreeNode("Health"))
	{
		UI::DrawDebug();
		ImGui::DragFloat("GaugeLerpSpeed", &gaugeLerpSpeed_);	//	補完する速さ
		
		//	HPの枠
		if (ImGui::TreeNode("HpFrame"))
		{
			hpFrame_->DrawDebug();
			ImGui::TreePop();
		}

		//	HPゲージ
		if (ImGui::TreeNode("Gauge"))
		{
			hpGauge_->DrawDebug();
			ImGui::TreePop();
		}

		//	後ろのゲージ
		if (ImGui::TreeNode("GaugeBack"))
		{
			hpGaugeBack_->DrawDebug();
			ImGui::TreePop();
		}

		ImGui::TreePop();
	}
}
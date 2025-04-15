#pragma once

#include "../../Game/UI/UIManager.h"
#include "../../Nova/Resources/Sprite.h"

//class UIManager;

class UI
{
public:
	UI(const UIManager::UIType& type);
	~UI(){}

	virtual void Initialize();
	virtual void Update(const float& elapsedTime);
	virtual void Render();
	virtual void DrawDebug();

	virtual void SetIsVisible(const bool& isVisible) = 0;
	bool GetIsVisible() { return isVisible_; }

	const UIManager::UIType GetUIType() { return type_; }

protected:
	bool isVisible_ = true;			//	UI•\Ž¦ƒtƒ‰ƒO

	UIManager::UIType type_ = {};	//	UI‚ÌŽí—Þ

};


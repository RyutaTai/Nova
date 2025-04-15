#pragma once

#include "UI.h"

//	‘€ìà–¾•\¦ƒNƒ‰ƒX
class UIInstructions :public UI
{
public:
	UIInstructions();
	~UIInstructions() {}

	void Initialize()override;
	void Update(const float& elapsedTime)override;
	void Render()override;
	void DrawDebug()override;

private:
	std::unique_ptr<Sprite> instruction_;	//	‘€ìà–¾

};


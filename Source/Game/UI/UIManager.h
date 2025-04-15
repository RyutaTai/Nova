#pragma once

#include <vector>
#include <set>

class UI;
class UIManager
{
public:
	//	UI‚Ìí—Ş
	enum class UIType
	{
		Instruction,	//	à–¾
		Health,			//	HP
		Tempo,			//	ƒeƒ“ƒ|
		Rhythm,			//	ƒŠƒYƒ€”»’è
		Max
	};

public:
	UIManager(){}
	~UIManager(){}

	static UIManager& Instance()
	{
		static UIManager uiManager;
		return uiManager;
	}

	void	Initialize();
	void	Update(const float& elpasedTime);
	void	Render();
	void	Finalize();
	void	DrawDebug();

	void	Register(UI* ui);
	void	Remove(UI* ui);
	void	RemoveFromType(const UIType& type);

	void	SetIsVisible(const bool& isVisible);

	UI*		GetUIFromNum(const int& num);
	UI*		GetUIFromType(const UIType& type);
	bool	ExistUI(const UIType& type);

private:
	std::vector<UI*> userInterfaces_;
	std::set<UI*>	generates_;
	std::set<UI*>	removes_;
	bool allIsVisible_ = true;

};


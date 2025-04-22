#pragma once

#include <vector>
#include <set>

class UI;
class UITempo;
class UIRank;
class UIManager
{
public:
	//	UIの種類
	enum class UIType
	{
		Instruction,	//	説明
		Health,			//	HP
		Tempo,			//	テンポ
		Rhythm,			//	リズム判定
		Rank,			//	ランク
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

	UI*			GetUIFromNum(const int& num);
	UI*			GetUIFromType(const UIType& type);

	void		SetIsVisible(const bool& isVisible);
	bool		ExistUI(const UIType& type);

	//	テンポUI
	void		RegisterUITempo(UITempo* uiTempo);
	UITempo*	GetUITempo();

	//	ランクUI
	void	RegisterUIRank(UIRank* uiRank);
	UIRank* GetUIRank();

private:
	std::vector<UI*> userInterfaces_;
	std::set<UI*>	generates_;
	std::set<UI*>	removes_;
	UITempo*		uiTempo_ = nullptr;		//	UITempo
	UIRank*			uiRank_ = nullptr;		//	UIRank
	bool			allIsVisible_ = true;

};


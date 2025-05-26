#pragma once

#include <vector>
#include <memory>

class UI;
class UITempo;
class UIRank;
class UIManager
{
public:
	UIManager(){}
	~UIManager(){}

	static UIManager& Instance()
	{
		static UIManager uiManager;
		return uiManager;
	}

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
	void	Initialize();
	void	Update(const float& elpasedTime);
	void	Render();
	void	Finalize();
	void	DrawDebug();

	void	Register(std::unique_ptr<UI> ui);
	void	Remove(UI* ui);		//	UIを削除リストに追加
	void	RemoveFromType(const UIType& type);	//	指定したタイプのUIをすべて削除リストに追加

	UI*			GetUIFromNum(const int& num);
	UI*			GetUIFromType(const UIType& type);

	void		SetIsVisible(const bool& isVisible);

	//	テンポUI
	void		RegisterUITempo(UITempo* uiTempo);
	UITempo*	GetUITempo();

	//	ランクUI
	void	RegisterUIRank(UIRank* uiRank);
	UIRank* GetUIRank();

private:
	std::vector<std::unique_ptr<UI>>	userInterfaces_;	//	UIManagerが管理・所有するUIをunique_ptrのvectorで管理(保持し続ける)
	std::vector<UI*>					generates_;			//	新しく追加されるUIの生ポインタをvectorで管理(一時的に保持)
	std::vector<UI*>					removes_;			//	削除されるUIの生ポインタをvectorで管理(一時的に保持)	
	UITempo*							uiTempo_ = nullptr;	//	UITempo
	UIRank*								uiRank_ = nullptr;	//	UIRank
	bool			allIsVisible_ = true;

};


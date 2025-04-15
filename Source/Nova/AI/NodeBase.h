#pragma once

#include <vector>
#include <string>

#include "BehaviorTree.h"
#include "ActionBase.h"

class JudgmentBase;
class BehaviorData;

//	メモリリーク調査用
#define debug_new new(_NORMAL_BLOCK,__FILE__,__LINE__)

//	ノード
class NodeBase
{
public:
	//	コンストラクタ
	NodeBase(std::string name,NodeBase* parent,NodeBase* sibling,const int& priority,
		BehaviorTree::SelectRule selectRule, JudgmentBase* judgment, ActionBase* action, const int& hierarchyNo, const bool& isForceExecution = false) :
		name_(name),parent_(parent),sibling_(sibling),priority_(priority),
		selectRule_(selectRule),judgment_(judgment),action_(action),hierarchyNo_(hierarchyNo),
		children_(NULL),isForceExecution_(isForceExecution)
	{
	}
	//	デストラクタ
	~NodeBase();
	//	名前ゲッター
	std::string GetName() { return name_; }
	//	親ノードゲッター
	NodeBase* GetParent() { return parent_; }
	//	子ノードゲッター
	NodeBase* GetChild(const int& index);
	//	子ノードゲッター(末尾)
	NodeBase* GetLastChild();
	//	子ノードゲッター(先頭)
	NodeBase* GetTopChild();
	//	兄弟ノードゲッター
	NodeBase* GetSibling() { return sibling_; }
	//	階層番号ゲッター
	int GetHirerchyNo() { return hierarchyNo_; }
	//	優先順位ゲッター
	int GetPriority() { return priority_; }
	//	親ノードセッター
	void SetParent(NodeBase* parent) { this->parent_ = parent; }
	//	子ノード追加
	void AddChild(NodeBase* child) { children_.push_back(child); }
	//	兄弟ノードセッター
	void SetSibling(NodeBase* sibling) {this->sibling_ = sibling;}
	//	行動データを持っているか
	bool HasAction() { return action_ != nullptr ? true : false; }
	//	行動データ取得
	ActionBase* GetAction()const { return action_; }
	//	実行可否判定
	bool Judgment();
	//	優先順位選択
	NodeBase* SelectPriority(std::vector<NodeBase*>* list);
	//	ランダム選択
	NodeBase* SelectRandom(std::vector<NodeBase*>* list);
	//	シーケンス選択
	NodeBase* SelectSequence(std::vector<NodeBase*>* list, BehaviorData* data);
	//	ノード検索
	NodeBase* SearchNode(std::string searchName);
	//	ノード推論
	NodeBase* Inference(Enemy* enemy, BehaviorData* data);
	//	実行
	ActionBase::State Run(const float& elapsedTime);

	//	----- 強制実行フラグ -----
	void SetIsForceExecution(const bool& isForceExecution) { isForceExecution_ = isForceExecution; }
	const bool IsForceExecution() const{ return isForceExecution_; }

public:
	std::vector<NodeBase*>		children_;		//	子ノード

protected:
	std::string					name_;			//	名前
	BehaviorTree::SelectRule	selectRule_;	//	選択ルール
	JudgmentBase*				judgment_;		//	判定クラス
	ActionBase*					action_;		//	実行クラス
	unsigned int				priority_;		//	優先順位
	NodeBase*					parent_;		//	親ノード
	NodeBase*					sibling_;		//	兄弟ノード
	int							hierarchyNo_;	//	階層番号

	bool						isForceExecution_ = false;	//	強制実行フラグ(他のノードが実行中でもJudgmentがtrueなら実行する)

};
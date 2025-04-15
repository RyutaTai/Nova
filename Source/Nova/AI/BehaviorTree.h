#pragma once

#include <string>

class ActionBase;
class JudgmentBase;
class NodeBase;
class BehaviorData;
class Enemy;

//	ビヘイビアツリー
class BehaviorTree
{
public:
	//	選択ルール
	enum class SelectRule
	{
		Non,				//	無い末端ノード用
		Priority,			//	優先順位
		Sequence,			//	シーケンス
		SequentialLooping,	//	シーケンシャルルーピング
		Random,				//	ランダム
	};

public:
	BehaviorTree()				:root_(nullptr) , owner_(nullptr) {}
	BehaviorTree(Enemy* enemy)	:root_(nullptr) , owner_(enemy){}
	~BehaviorTree();

	void DrawDebug();

	//	実行ノードを推論する
	NodeBase* ActiveNodeInference(BehaviorData* data);

	//	シーケンスノードから推論開始
	NodeBase* SequenceBack(NodeBase* sequenceNode, BehaviorData* data);

	//	ノード追加
	void AddNode(const std::string& parentName, const std::string& entryName, const int& priority, const SelectRule& selectRule, JudgmentBase* judgment, ActionBase* action, const bool& isForceExecution = false);

	//	実行
	NodeBase* Run(NodeBase* actionNode, BehaviorData* data, const float& elapsedTime);

private:
	//	ノード全削除
	void NodeAllClear(NodeBase* delNode);
private:
	//	ルートノード
	NodeBase* root_;
	Enemy* owner_;

};

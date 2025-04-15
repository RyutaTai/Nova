#include "BehaviorTree.h"

#include "ActionBase.h"
#include "NodeBase.h"
#include "JudgmentBase.h"
#include "BehaviorData.h"
#include "../../Game/Enemy.h"

//	デストラクタ
BehaviorTree::~BehaviorTree()
{
	NodeAllClear(root_);
}

//	ノード追加
void BehaviorTree::AddNode(const std::string& parentName, const std::string& entryName, const int& priority, const SelectRule& selectRule, JudgmentBase* judgment, ActionBase* action,const bool& isForceExecution)
{
	if (parentName != "")
	{
		NodeBase* parentNode = root_->SearchNode(parentName);

		if (parentNode != nullptr)
		{
			NodeBase* sibling = parentNode->GetLastChild();
			NodeBase* addNode = new NodeBase(entryName, parentNode, sibling, priority, selectRule, judgment, action, parentNode->GetHirerchyNo() + 1, isForceExecution);

			parentNode->AddChild(addNode);
		}
	}
	else 
	{
		if (root_ == nullptr)
		{
			root_ = new NodeBase(entryName, nullptr, nullptr, priority, selectRule, judgment, action, 1, isForceExecution);
		}
	}
}

//	推論
NodeBase* BehaviorTree::ActiveNodeInference(BehaviorData* data)
{
	//	データをリセットして開始
	data->Initialize();
	return root_->Inference(owner_, data);
}

//	シーケンスノードからの推論開始
NodeBase* BehaviorTree::SequenceBack(NodeBase* sequenceNode, BehaviorData* data)
{
	return sequenceNode->Inference(owner_, data);
}

//	ノード実行
NodeBase* BehaviorTree::Run(NodeBase* actionNode, BehaviorData* data, const float& elapsedTime)
{
	//	ノード実行
	ActionBase::State state = actionNode->Run(elapsedTime);

	//	正常終了
	if (state == ActionBase::State::Complete)
	{
		//	シーケンスの途中かを判断
		NodeBase* sequenceNode = data->PopSequenceNode();

		//	途中じゃないなら終了
		if (sequenceNode == nullptr)
		{
			return nullptr;
		}
		else
		{
			//	途中ならそこから始める
			return SequenceBack(sequenceNode, data);
		}
		//	失敗は終了
	}
	else if (state == ActionBase::State::Failed)
	{
		return nullptr;
	}

	//	現状維持
	return actionNode;
}

//	登録されたノードを全て削除する
void BehaviorTree::NodeAllClear(NodeBase* delNode)
{
	size_t count = delNode->children_.size();
	if (count > 0)
	{
		for (NodeBase* node :delNode->children_)
		{
			NodeAllClear(node);
		}
		delete delNode;
	}
	else
	{
		delete delNode;
	}
}

void BehaviorTree::DrawDebug()
{
	//	登録されているノードのImGui描画
	size_t count = root_->children_.size();
	if (count > 0)
	{
		for (NodeBase* node : root_->children_)
		{
			if (node->GetAction() != nullptr)
				node->GetAction()->DrawDebug();

			for (NodeBase* n : node->children_)
			{
				if (n->GetAction() != nullptr)
					n->GetAction()->DrawDebug();
			}

		}
	}

}


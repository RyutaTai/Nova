#include "NodeBase.h"

#include "JudgmentBase.h"
#include "BehaviorData.h"
#include "ActionBase.h"
#include "../Others/MathHelper.h"

//	デストラクタ
NodeBase::~NodeBase()
{
	delete judgment_;
	delete action_;
}

//	子ノードゲッター
NodeBase* NodeBase::GetChild(const int& index)
{
	if (children_.size() <= index)//修正
	{
		return nullptr;
	}
	return children_.at(index);
}

//	子ノードゲッター(末尾)
NodeBase* NodeBase::GetLastChild()
{
	if (children_.size() == 0)
	{
		return nullptr;
	}

	return children_.at(children_.size() - 1);
}

//	子ノードゲッター(先頭)
NodeBase* NodeBase::GetTopChild()
{
	if (children_.size() == 0)
	{
		return nullptr;
	}

	return children_.at(0);
}

//	ノード検索
NodeBase* NodeBase::SearchNode(std::string searchName)
{
	//	名前が一致
	if (name_ == searchName)
	{
		return this;
	}
	else 
	{
		//	子ノードで検索
		for (auto it = children_.begin(); it != children_.end(); it++)
		{
			NodeBase* ret = (*it)->SearchNode(searchName);

			if (ret != nullptr)
			{
				return ret;
			}
		}
	}

	return nullptr;
}

//	ノード推論
NodeBase* NodeBase::Inference(Enemy* enemy, BehaviorData* data)
{
	std::vector<NodeBase*> list;
	NodeBase* result = nullptr;

	for (int i = 0; i < children_.size(); i++)
	{
		if (children_.at(i)->judgment_ != nullptr)
		{
			if (children_.at(i)->judgment_->Judgment())
			{
				list.emplace_back(children_.at(i));
			}
		}
		else 
		{
			//	判定クラスがなければ無条件に追加
			list.emplace_back(children_.at(i));
		}
	}

	//	選択ルールでノード決め
	switch (selectRule_)
	{
		//	優先順位
	case BehaviorTree::SelectRule::Priority:
		result = SelectPriority(&list);
		break;
		//	ランダム
	case BehaviorTree::SelectRule::Random:
		result = SelectRandom(&list);
		break;
		//	シーケンス
	case BehaviorTree::SelectRule::Sequence:
	case BehaviorTree::SelectRule::SequentialLooping:
		result = SelectSequence(&list, data);
		break;
	}

	if (result != nullptr)
	{
		//	行動があれば終了
		if (result->HasAction() == true)
		{
			return result;
		}
		else 
		{
			//	決まったノードで推論開始
			result = result->Inference(enemy, data);
		}
	}

	return result;
}

//	優先順位でノード選択
NodeBase* NodeBase::SelectPriority(std::vector<NodeBase*>* list)
{
	NodeBase* selectNode = nullptr;
	int priority = INT_MAX;

	//	listに実行可能なノードが登録されている
	//	listに登録されたノードの中で一番優先順位が高いものを探してselectNodeに格納(数が小さい程優先は高い）
	for (NodeBase* node : *list)
	{
		if (priority >node->GetPriority())	//	前回より優先順位が高かったら
		{
			priority = node->GetPriority();
			selectNode = node;
		}
	}
	return selectNode;
}

//	ランダムでノード選択
NodeBase* NodeBase::SelectRandom(std::vector<NodeBase*>* list)
{
	int selectNo = 0;
	//	listのサイズで乱数を取得してselectNoに格納
	selectNo = Mathf::RandomRange(0, static_cast<int>(list->size()) - 1);
	
	//	listのselectNo番目の実態をリターン
	return (*list).at(selectNo);
}

//	シーケンス・シーケンシャルルーピングでノード選択
NodeBase* NodeBase::SelectSequence(std::vector<NodeBase*>* list, BehaviorData* data)
{
	int step = 0;

	//	この中間ノードが次に実行すべきstepを取得する
	step = data->GetSequenceStep(name_);

	//	中間ノードに登録されているノード数以上の場合、
	if (step >= children_.size())
	{
		//	ルールによってシーケンシャルの処理を切り替える（ルールはthisで参照可能）
		//	ルールがBehaviorTree::SelectRule::SequentialLoopingのときは最初から実行するため、stepに0を代入
		//	ルールがBehaviorTree::SelectRule::Sequenceのときは次に実行できるノードがないため、nullptrをリターン
		if (selectRule_ == BehaviorTree::SelectRule::SequentialLooping)
		{
			step = 0;
		}
		else if (selectRule_ == BehaviorTree::SelectRule::Sequence)
		{
			return nullptr;
		}

	}
	//	実行可能リストに登録されているデータの数だけループを行う
#if 1
	for (auto itr = list->begin(); itr != list->end(); itr++)
	{
		//	子ノードが実行可能リストに含まれているか
		if (children_.at(step)->GetName() == (*itr)->GetName())
		{
			//	現在の実行ノードをスタックに保存、次に実行するステップの保存を行った後、
			//	現在のステップ番号のノードをreturn
			//	スタックにはdata->PushSequenceNode関数を使用する。保存するデータは実行中の中間ノードであるthis。
			//	次に中間ノードと「次のステップ数」を保存する
			//	　ステップ数の保存にはdata->SetSequenceStep関数を使用する事。
			//	　引数はこの中間ノードの名前(this->name)とステップ数+1(step + 1)
			//	ステップ番号目の子ノードを実行ノードとしてreturn
			data->PushSequenceNode(this);
			data->SetSequenceStep(this->name_, step + 1);
			return children_.at(step);
		}
	}
#else
	for (NodeBase* node : *list)
	{
		//	子ノードが実行可能リストに含まれているか
		if (children.at(step)->GetName() == node->GetName())
		{
			data->PushSequenceNode(this);
			data->SetSequenceStep(this->name, step + 1);
			return children.at(step);
		}
	}
#endif

	//	指定された中間ノードに実行可能ノードがないのでnullptrをreturn
	return nullptr;

}

//	判定
bool NodeBase::Judgment()
{
	//	judgmentがあるか判断。あればメンバ関数Judgment()実行した結果をreturn
	{
		return judgment_->Judgment();
	}
	return true;
}

//	ノード実行
ActionBase::State NodeBase::Run(const float& elapsedTime)
{
	//	actionがあるか判断。あればメンバ関数Run()実行した結果をreturn
	if (action_ != nullptr)
	{
		return action_->Run(elapsedTime);
	}

	return ActionBase::State::Failed;
}

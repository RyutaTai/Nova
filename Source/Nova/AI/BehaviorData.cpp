#include "BehaviorData.h"

#include "NodeBase.h"
#include "../../External/imgui/imgui.h"

//	初期化
void BehaviorData::Initialize()
{
	runSequenceStepMap_.clear();
	while (sequenceStack_.size() > 0)
	{
		sequenceStack_.pop();
	}
}

//	シーケンスノードのポップ
NodeBase* BehaviorData::PopSequenceNode()
{
	//	空ならnullptrを返す
	if (sequenceStack_.empty() != 0)
	{
		return nullptr;
	}
	NodeBase* node = sequenceStack_.top();
	if (node != nullptr)
	{
		//	取り出したデータを削除
		sequenceStack_.pop();
	}
	return node;
}

//	シーケンスステップのゲッター
int BehaviorData::GetSequenceStep(const std::string& name)
{
	if (runSequenceStepMap_.count(name) == 0)
	{
		runSequenceStepMap_.insert(std::make_pair(name,0));
	}

	return runSequenceStepMap_.at(name);
}

//	シーケンスステップのセッター
void BehaviorData::SetSequenceStep(const std::string& name, const int& step)
{
	runSequenceStepMap_.at(name) = step;
}

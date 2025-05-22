#include "TimeRangeJudge.h"

#include "../../imgui/imgui.h"

TimeRangeJudge::TimeRangeJudge()
{
	minTime_ = 0.0f;
	maxTime_ = FLT_MAX;
	name_ = {};
}

//	Žw’è‚³‚ê‚½ŽžŠÔ‚ª”ÍˆÍ“à‚É‚ ‚é‚©‚ð”»’è
const bool TimeRangeJudge::IsWithinRange(const float& time)const
{
	return (minTime_ < time) && (time < maxTime_);
}

//	ŽžŠÔ”»’è‚ðÝ’è‚·‚é
void TimeRangeJudge::SetRange(const float& minTime,const float& maxTime)
{
	minTime_ = minTime; 
	maxTime_ = maxTime;
}

void TimeRangeJudge::DrawDebug()
{
	if (ImGui::TreeNode(name_.c_str()))
	{
		ImGui::DragFloat("MinTime", &minTime_, 0.1f);
		ImGui::DragFloat("MaxTime", &maxTime_, 0.1f);

		ImGui::TreePop();
	}
}
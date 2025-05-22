#include "JudgeTime.h"

#include "../../imgui/imgui.h"

JudgeTime::JudgeTime()
{
	minJudgeTime_ = 0.0f;
	maxJudgeTime_ = FLT_MAX;
	name_ = {};
}

//	time‚ª”»’èŽžŠÔ‚Ì”ÍˆÍ‚É“ü‚Á‚Ä‚¢‚é‚©
const bool JudgeTime::IsJudgeFlag(const float& time)const
{
	return (minJudgeTime_ < time) && (time < maxJudgeTime_);
}

//	”»’èŽžŠÔÝ’è
void JudgeTime::SetJudgeTime(const float& minTime,const float& maxTime)
{
	minJudgeTime_ = minTime; 
	maxJudgeTime_ = maxTime;
}

void JudgeTime::DrawDebug()
{
	if (ImGui::TreeNode(name_.c_str()))
	{
		ImGui::DragFloat("MinTime", &minJudgeTime_, 0.1f);
		ImGui::DragFloat("MaxTime", &maxJudgeTime_, 0.1f);

		ImGui::TreePop();
	}
}
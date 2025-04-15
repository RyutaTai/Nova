#pragma once

#include <float.h>
#include <string>

class JudgeTime
{
public:
	JudgeTime();
	~JudgeTime(){}

	void DrawDebug();
	
	const bool IsJudgeFlag(const float& time)const;	//	時間がminJudgeTime_とmaxJudgeTime_の間にあるか判断

	void		SetJudgeTime(const float& minTime, const float& maxTime);
	void		SetMinJudgeTime(const float& minTime) { minJudgeTime_ = minTime; }		//	判定時間の下限値設定
	const float GetMinTime()		const { return minJudgeTime_; }
	
	void		SetMaxJudgeTime(const float& maxTime) { maxJudgeTime_ = maxTime; }		//	判定時間の上限値設定
	const float GetMaxTime()		const { return maxJudgeTime_; }

	const float CalcJudgeDuration()	const { return maxJudgeTime_ - minJudgeTime_; }		//	判定時間の長さ取得

	void				SetName(const std::string& name) { name_ = name; }
	const std::string	GetName()const { return name_; }

private:
	float minJudgeTime_ = 0.0f;		//	判定する時間の下限値
	float maxJudgeTime_ = FLT_MAX;	//	判定する時間の上限値

	std::string name_	= {};		//	名前

};


#pragma once

#include <float.h>
#include <string>

//	時間の範囲(上限と下限)を指定して、渡された値がその範囲内かどうかを判定するクラス
class TimeRangeJudge
{
public:
	TimeRangeJudge();
	~TimeRangeJudge(){}

	void DrawDebug();
	
	//	----- 指定された時間が範囲内にあるかを判定 -----
	const bool IsWithinRange(const float& time)const;

	//	----- 時間範囲 -----
	void		SetRange(const float& minTime, const float& maxTime);
	void		SetMaxTime(const float& maxTime)	{ maxTime_ = maxTime; }
	const float GetMaxTime()const					{ return maxTime_; }
	void		SetMinTime(const float& minTime)	{ minTime_ = minTime; }
	const float GetMinTime()const					{ return minTime_; }

	//	----- 範囲の長さを取得 -----
	const float GetDuration()	const { return maxTime_ - minTime_; }	

	//	----- 名前 -----
	void				SetName(const std::string& name) { name_ = name; }
	const std::string	GetName()const					 { return name_; }

private:
	float maxTime_ = FLT_MAX;	//	判定する時間の上限値
	float minTime_ = 0.0f;		//	判定する時間の下限値

	std::string name_	= {};	//	名前

};


#pragma once

#include <windows.h>

class HighResolutionTimer
{
public:
	HighResolutionTimer()
	{
		LONGLONG countsPerSec;
		QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&countsPerSec));
		secondsPerCount_ = 1.0 / static_cast<double>(countsPerSec);

		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&thisTime_));
		baseTime_ = thisTime_;
		lastTime_ = thisTime_;
	}
	~HighResolutionTimer() = default;
	HighResolutionTimer(const HighResolutionTimer&) = delete;
	HighResolutionTimer& operator=(const HighResolutionTimer&) = delete;
	HighResolutionTimer(HighResolutionTimer&&) noexcept = delete;
	HighResolutionTimer& operator=(HighResolutionTimer&&) noexcept = delete;

	// Returns the total time elapsed since Reset() was called, NOT counting any
	// time when the clock is stopped.
	float TimeStamp() const  // in seconds
	{
		// If we are stopped, do not count the time that has passed since we stopped.
		// Moreover, if we previously already had a pause, the distance 
		// stop_time - base_time includes paused time, which we do not want to count.
		// To correct this, we can subtract the paused time from mStopTime:  
		//
		//                     |<--paused_time-->|
		// ----*---------------*-----------------*------------*------------*------> time
		//  base_time       stop_time        start_time     stop_time    this_time

		if (stopped_)
		{
			return static_cast<float>(((stopTime_ - pausedTime_) - baseTime_) * secondsPerCount_);
		}

		// The distance this_time - mBaseTime includes paused time,
		// which we do not want to count.  To correct this, we can subtract 
		// the paused time from this_time:  
		//
		//  (this_time - paused_time) - base_time 
		//
		//                     |<--paused_time-->|
		// ----*---------------*-----------------*------------*------> time
		//  base_time       stop_time        start_time     this_time
		else
		{
			return static_cast<float>(((thisTime_ - pausedTime_) - baseTime_) * secondsPerCount_);
		}
	}

	float GetDeltaTime() const  // in seconds
	{
		return static_cast<float>(deltaTime_);
	}

	double GetDoubleDeltaTime()const
	{
		return deltaTime_;
	}

	void Reset() // Call before message loop.
	{
		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&thisTime_));
		baseTime_ = thisTime_;
		lastTime_ = thisTime_;

		stopTime_ = 0;
		stopped_ = false;
	}

	void Start() // Call when unpaused.
	{
		LONGLONG startTime;
		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&startTime));

		// Accumulate the time elapsed between stop and start pairs.
		//
		//                     |<-------d------->|
		// ----*---------------*-----------------*------------> time
		//  base_time       stop_time        start_time     
		if (stopped_)
		{
			pausedTime_ += (startTime - stopTime_);
			lastTime_ = startTime;
			stopTime_ = 0;
			stopped_ = false;
		}
	}

	void Stop() // Call when paused.
	{
		if (!stopped_)
		{
			QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&stopTime_));
			stopped_ = true;
		}
	}

	void Tick() // Call every frame.
	{
		if (stopped_)
		{
			deltaTime_ = 0.0;
			return;
		}

		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&thisTime_));
		// Time difference between this frame and the previous.
		deltaTime_ = (thisTime_ - lastTime_) * secondsPerCount_;
		if (deltaTime_ > 1.0f)
		{
			deltaTime_ = 1.0f;
		}

		// Prepare for next frame.
		lastTime_ = thisTime_;

		// Force nonnegative.  The DXSDK's CDXUTTimer mentions that if the 
		// processor goes into a power save mode or we get shuffled to another
		// processor, then mDeltaTime can be negative.
		if (deltaTime_ < 0.0)
		{
			deltaTime_ = 0.0;
		}
	}

private:
	LONGLONG baseTime_{ 0LL };
	LONGLONG pausedTime_{ 0LL };
	LONGLONG stopTime_{ 0LL };
	LONGLONG lastTime_{ 0LL };
	LONGLONG thisTime_{ 0LL };

	double secondsPerCount_{ 0.0 };
	double deltaTime_{ 0.0 };

	bool stopped_{ false };

};

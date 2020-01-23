#pragma once

#include "ZeoEngine.h"

enum class TimerState
{
	Running,
	StartNextLoop,
	Finished,
};

class Timer
{
	friend class TimerManager;

public:
	Timer() = default;
	Timer(float startTime, float duration, std::function<void()> callback, int loopCount);

	void SetStartTime(float startTime) { m_StartTime = startTime; }

	TimerState OnUpdate(float currentTime);

private:
	float m_StartTime = 0.0f;
	float m_Duration = 0.0f;
	std::function<void()> m_Callback;
	int m_LoopCount = 0;
	
};

class TimerManager
{
public:
	/**
	 * Start a countdown timer and call the callback when done.
	 * @param loopCount - <= 0: loop infinitely, 1: loop once
	 */
	void SetTimer(float duration, std::function<void()> callback, int loopCount = 1);

	void OnUpdate(ZeoEngine::DeltaTime dt);

private:
	std::vector<Timer> m_Timers;
	float m_Time = 0.0f;
};

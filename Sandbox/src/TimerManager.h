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
	Timer(float startTime, float duration, std::function<void()> callback, int32_t loopCount, float firstDelay);

	void SetStartTime(float startTime) { m_StartTime = startTime; }

	TimerState OnUpdate(float currentTime);

private:
	float m_StartTime = 0.0f;
	float m_Duration = 0.0f;
	std::function<void()> m_Callback;
	int32_t m_LoopCount = 0;
	float m_FirstDelay;
	bool m_bFirstLoop = true;
};

class TimerManager
{
public:
	/**
	 * Start a countdown timer and call the callback when done.
	 * @param loopCount - <= 0: loop infinitely, 1: loop once
	 * @param bFirstDelay - Time before invoking callback for the first time. If < 0.0, duration will be used.
	 */
	void SetTimer(float duration, std::function<void()> callback, int32_t loopCount = 1, float firstDelay = -1.0f);

	void OnUpdate(ZeoEngine::DeltaTime dt);

private:
	std::vector<Timer> m_Timers;
	float m_Time = 0.0f;
};

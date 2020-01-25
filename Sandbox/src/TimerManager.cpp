#include "TimerManager.h"

Timer::Timer(float startTime, float duration, std::function<void()> callback, int loopCount, float firstDelay)
	: m_StartTime(startTime)
	, m_Duration(duration)
	, m_Callback(callback)
	, m_LoopCount(loopCount)
	, m_FirstDelay(firstDelay)
{
}

TimerState Timer::OnUpdate(float currentTime)
{
	float duration = m_bFirstLoop ? (m_FirstDelay >= 0.0f ? m_FirstDelay : m_Duration) : m_Duration;
	if (currentTime - m_StartTime >= duration)
	{
		// If callback function is called inside this method,
		// for situations where there is another looping SetTimer() in the callback,
		// when this method returns is not guaranteed (as this is not an async call)
		--m_LoopCount;
		if (m_LoopCount == 0)
		{
			m_bFirstLoop = false;
			return TimerState::Finished;
		}
		else
		{
			m_bFirstLoop = false;
			return TimerState::StartNextLoop;
		}
	}
	m_bFirstLoop = false;
	return TimerState::Running;
}

void TimerManager::SetTimer(float duration, std::function<void()> callback, int loopCount, float firstDelay)
{
	//ZE_TRACE("Creating new timer...");
	m_Timers.emplace_back(m_Time, duration, callback, loopCount, firstDelay);
}

void TimerManager::OnUpdate(ZeoEngine::DeltaTime dt)
{
	m_Time += dt;

	for (uint32_t i = 0; i < m_Timers.size(); ++i)
	{
		switch (m_Timers[i].OnUpdate(m_Time))
		{
			case TimerState::Running:
				break;
			case TimerState::StartNextLoop:
				//ZE_TRACE("Starting next timer loop...");
				m_Timers[i].SetStartTime(m_Time);
				m_Timers[i].m_Callback();
				break;
			case TimerState::Finished:
				m_Timers[i].m_Callback();
				//ZE_TRACE("Destroying finished timer...");
				m_Timers.erase(m_Timers.begin() + i);
				break;
			default:
				break;
		}
	}
}

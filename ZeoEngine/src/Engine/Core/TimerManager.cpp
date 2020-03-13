#include "ZEpch.h"
#include "Engine/Core/TimerManager.h"
#include "Engine/GameFramework/GameObject.h"

namespace ZeoEngine {

	Timer::Timer(float startTime, float duration, GameObject* outerObject, TimerFn callback, int32_t loopCount, float firstDelay, const TimerPlaceholder&)
		: m_StartTime(startTime)
		, m_Duration(duration)
		, m_OuterObject(outerObject)
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

	void TimerManager::SetTimer(float duration, GameObject* outerObject, TimerFn callback, int32_t loopCount, float firstDelay)
	{
		//ZE_CORE_TRACE("Creating new timer...");
		m_Timers.emplace_back(m_Time, duration, outerObject, callback, loopCount, firstDelay, Timer::TimerPlaceholder());
	}

	void TimerManager::OnUpdate(DeltaTime dt)
	{
		m_Time += dt;

		for (uint32_t i = 0; i < m_Timers.size(); ++i)
		{
			GameObject* outer = m_Timers[i].m_OuterObject;
			if (outer && outer->IsPendingDestroy())
			{
				//ZE_CORE_TRACE("Destroying dangling timer...");
				m_Timers.erase(m_Timers.cbegin() + i);
				continue;
			}

			switch (m_Timers[i].OnUpdate(m_Time))
			{
			case TimerState::Running:
				break;
			case TimerState::StartNextLoop:
				//ZE_CORE_TRACE("Starting next timer loop...");
				m_Timers[i].m_StartTime = m_Time;
				m_Timers[i].m_Callback();
				break;
			case TimerState::Finished:
				m_Timers[i].m_Callback();
				//ZE_CORE_TRACE("Destroying finished timer...");
				m_Timers.erase(m_Timers.cbegin() + i);
				break;
			default:
				break;
			}
		}
	}

	void TimerManager::CleanUp()
	{
		m_Timers.clear();
	}

}

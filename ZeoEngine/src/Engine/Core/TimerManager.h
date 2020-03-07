#pragma once

#include "Engine/Core/DeltaTime.h"

namespace ZeoEngine {

	class GameObject;

	enum class TimerState
	{
		Running,
		StartNextLoop,
		Finished,
	};

	using TimerFn = std::function<void()>;

	class Timer
	{
		friend class TimerManager;
		
		/** A dummy class used to prevent public Timer constructor from being called outside. */
		class TimerPlaceholder
		{
		public:
			TimerPlaceholder() = default;

			TimerPlaceholder(const TimerPlaceholder&) = delete;
			TimerPlaceholder& operator=(const TimerPlaceholder&) = delete;
		};

	private:
		Timer() = default;
	public:
		// Note: We make it public as std::vector::emplace_back() will need to access this
		Timer(float startTime, float duration, GameObject* outerObject, TimerFn callback, int32_t loopCount, float firstDelay, const TimerPlaceholder&);

	private:
		TimerState OnUpdate(float currentTime);

	private:
		float m_StartTime = 0.0f;
		float m_Duration = 0.0f;
		GameObject* m_OuterObject;
		TimerFn m_Callback;
		int32_t m_LoopCount = 0;
		float m_FirstDelay;
		bool m_bFirstLoop = true;
	};

	class TimerManager
	{
		friend class GameLayer;

	private:
		explicit TimerManager(const std::string& name)
		{
			ZE_CORE_TRACE("{0} timer manager initialized!", name);
		}

	public:
		TimerManager(const TimerManager&) = delete;
		TimerManager& operator=(const TimerManager&) = delete;

		/**
		 * Start a countdown timer and call the callback at a set interval.
		 * @param duration - Time (in seconds) before invoking callback
		 * @param outerObject - GameObject which calls this method. Passing nullptr if caller is not a GameObject
		 * @param callback - Function to invoke when timer fires
		 * @param loopCount - <= 0: loop infinitely, 1: loop once
		 * @param bFirstDelay - Time (in seconds) before invoking callback for the first time. If < 0.0, duration will be used
		 */
		void SetTimer(float duration, GameObject* outerObject, TimerFn callback, int32_t loopCount = 1, float firstDelay = -1.0f);

	private:
		void OnUpdate(DeltaTime dt);

	private:
		std::vector<Timer> m_Timers;
		float m_Time = 0.0f;
	};

}

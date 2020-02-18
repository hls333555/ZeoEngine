#pragma once

#include <chrono>
#include <iostream>

namespace ZeoEngine
{
	class BenchmarkTimer
	{
	public:
		BenchmarkTimer(bool bAutoPrint = true)
			: m_bAutoPrint(bAutoPrint)
		{
			m_StartTimePoint = std::chrono::high_resolution_clock::now();
		}

		~BenchmarkTimer()
		{
			auto endTimePoint = std::chrono::high_resolution_clock::now();
			
			auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimePoint).time_since_epoch().count();
			auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimePoint).time_since_epoch().count();
			
			auto duration_Micro = end - start;
			double duration_Milli = duration_Micro * 0.001;
			m_Duration = duration_Milli * 0.001;

			if (m_bAutoPrint)
			{
				std::cout << "Duration: " << duration_Milli << "ms" << std::endl;
			}
		}

		double GetDuration() const { return m_Duration; }

	private:
		std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTimePoint;
		bool m_bAutoPrint;
		double m_Duration = 0.0;

	};
}

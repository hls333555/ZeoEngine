#pragma once

#include <chrono>
#include <iostream>

namespace ZeoEngine {

	#define BEGIN_BENCHMARK() BenchmarkTimer bt;
	#define END_BENCHMARK() std::cout << bt.ElapsedMillis() << std::endl;

	class BenchmarkTimer
	{
	public:
		BenchmarkTimer()
		{
			Reset();
		}

		void Reset()
		{
			m_Start = std::chrono::high_resolution_clock::now();
		}

		float Elapsed()
		{
			return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - m_Start).count() * 0.001f * 0.001f * 0.001f;
		}

		float ElapsedMillis()
		{
			return Elapsed() * 1000.0f;
		}

	private:
		std::chrono::time_point<std::chrono::high_resolution_clock> m_Start;

	};
}

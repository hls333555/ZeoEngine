#pragma once

#include "Core.h"

#include "spdlog/spdlog.h"
// Support for logging for custom types
#include "spdlog/fmt/ostr.h"

namespace ZeoEngine {

	class ZE_API Log
	{
	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;

	};

}

// Core log macros
#define ZE_CORE_TRACE(...) ::ZeoEngine::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define ZE_CORE_INFO(...) ::ZeoEngine::Log::GetCoreLogger()->info(__VA_ARGS__)
#define ZE_CORE_WARN(...) ::ZeoEngine::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define ZE_CORE_ERROR(...) ::ZeoEngine::Log::GetCoreLogger()->error(__VA_ARGS__)
#define ZE_CORE_CRITICAL(...) ::ZeoEngine::Log::GetCoreLogger()->critical(__VA_ARGS__)

// Client log macros
#define ZE_TRACE(...) ::ZeoEngine::Log::GetClientLogger()->trace(__VA_ARGS__)
#define ZE_INFO(...) ::ZeoEngine::Log::GetClientLogger()->info(__VA_ARGS__)
#define ZE_WARN(...) ::ZeoEngine::Log::GetClientLogger()->warn(__VA_ARGS__)
#define ZE_ERROR(...) ::ZeoEngine::Log::GetClientLogger()->error(__VA_ARGS__)
#define ZE_CRITICAL(...) ::ZeoEngine::Log::GetClientLogger()->critical(__VA_ARGS__)

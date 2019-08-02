#pragma once

#include "Core.h"

#include "spdlog/spdlog.h"
// Support for logging for custom types
#include "spdlog/fmt/ostr.h"

namespace HBestEngine
{
	class HBE_API Log
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
#define HBE_CORE_TRACE(...) ::HBestEngine::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define HBE_CORE_INFO(...) ::HBestEngine::Log::GetCoreLogger()->info(__VA_ARGS__)
#define HBE_CORE_WARN(...) ::HBestEngine::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define HBE_CORE_ERROR(...) ::HBestEngine::Log::GetCoreLogger()->error(__VA_ARGS__)
#define HBE_CORE_FATAL(...) ::HBestEngine::Log::GetCoreLogger()->critical(__VA_ARGS__)

// Client log macros
#define HBE_TRACE(...) ::HBestEngine::Log::GetClientLogger()->trace(__VA_ARGS__)
#define HBE_INFO(...) ::HBestEngine::Log::GetClientLogger()->info(__VA_ARGS__)
#define HBE_WARN(...) ::HBestEngine::Log::GetClientLogger()->warn(__VA_ARGS__)
#define HBE_ERROR(...) ::HBestEngine::Log::GetClientLogger()->error(__VA_ARGS__)
#define HBE_FATAL(...) ::HBestEngine::Log::GetClientLogger()->critical(__VA_ARGS__)

#include "ZEpch.h"
#include "Engine/Core/Log.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#if WITH_EDITOR
#include "Engine/Core/EditorLog.h"
#endif // WITH_EDITOR

namespace ZeoEngine {

	Ref<spdlog::logger> Log::s_CoreLogger;
	Ref<spdlog::logger> Log::s_ClientLogger;

	void Log::Init()
	{
		std::vector<spdlog::sink_ptr> sinks;
#if WITH_EDITOR
		sinks.push_back(std::make_shared<spdlog::sinks::editorlog_sink_mt>());
#else
		sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
#endif // WITH_EDITOR
		sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("log.log", true));

		s_CoreLogger = std::make_shared<spdlog::logger>("ZE", begin(sinks), end(sinks));
		spdlog::register_logger(s_CoreLogger);
		// Only messages whose level >= log_level will be logged to the console, in this case, all types of messages will be logged
		s_CoreLogger->set_level(spdlog::level::trace);
		s_CoreLogger->flush_on(spdlog::level::trace);

		s_ClientLogger = std::make_shared<spdlog::logger>("APP", begin(sinks), end(sinks));
		spdlog::register_logger(s_ClientLogger);
		s_ClientLogger->set_level(spdlog::level::trace);
		s_ClientLogger->flush_on(spdlog::level::trace);

		// Set the global pattern, check https://github.com/gabime/spdlog/wiki/3.-Custom-formatting for more information about formatting
		// Call this after loggers being registered
		spdlog::set_pattern("%^[%T] [%l] %n: %v%$");
	}

}

#include "Engine/Core/Log.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#include "Widgets/EditorConsole.h"

namespace ZeoEngine {

	void Log::Init()
	{
		std::vector<spdlog::sink_ptr> sinks;

		sinks.push_back(CreateRef<spdlog::sinks::editorlog_sink_mt>());
		//sinks.push_back(CreateRef<spdlog::sinks::stdout_color_sink_mt>()); // Put this to RuntimeLog
		sinks.push_back(CreateRef<spdlog::sinks::basic_file_sink_mt>("log.log", true));

		s_CoreLogger = CreateRef<spdlog::logger>("ZE", begin(sinks), end(sinks));
		spdlog::register_logger(s_CoreLogger);
		// Only messages whose level >= log_level will be logged to the console, in this case, all types of messages will be logged
		s_CoreLogger->set_level(spdlog::level::trace);
		s_CoreLogger->flush_on(spdlog::level::trace);

		s_ClientLogger = CreateRef<spdlog::logger>("APP", begin(sinks), end(sinks));
		spdlog::register_logger(s_ClientLogger);
		s_ClientLogger->set_level(spdlog::level::trace);
		s_ClientLogger->flush_on(spdlog::level::trace);

		// Set the global pattern, check https://github.com/gabime/spdlog/wiki/3.-Custom-formatting for more information about formatting
		// Call this after loggers being registered
		spdlog::set_pattern("%^[%T] [%l] %n: %v%$");

		ZE_CORE_TRACE("Log initialized");
	}

}

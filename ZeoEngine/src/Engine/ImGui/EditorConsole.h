#pragma once

#include <imgui.h>
#include <spdlog/sinks/base_sink.h>

#include "Engine/ImGui/TextFilter.h"

namespace ZeoEngine {

	struct EditorConsole
	{
		static EditorConsole s_Instance;

		ImGuiTextBuffer     LogBuffer;
		TextFilter			LogFilter;
		ImVector<int>       LogLineOffsets;	// Index to lines offset. We maintain this with AddLog() calls, allowing us to have a random access on lines
		ImVector<int>		LogLevels;
		ImVector<ImVec4>	LogColors;
		bool                bAutoScroll;	// Keep scrolling if already at the bottom
		bool				bEnableLogLevelFilters[6];

		TextFilter			CommandFilter;
		float				MaxCommandLineWidth = 0.0f;
		struct CommandInputState
		{
			bool bIsPopupOpen = false;
			int  ActiveIdx = -1;			// Index of currently 'active' popup command by use of up/down keys. Ranges from [0, FilteredCommandLines.Size)
			int  ClickedIdx = -1;			// Index of popup command clicked with the mouse. Ranges from [0, FilteredCommandLines.Size)
			bool bIsAutoCompletion = false; // Flag set when a popup command is tabbed or clicked
			bool bSelectionChanged = false;	// Flag to help focus the correct item when selecting active item
			ImGuiTextBuffer CommandBuffer; // TODO: Redundant? See Console::m_Commands
			ImVector<int> CommandLineOffsets; // Stores line offset of command
			ImVector<int> FilteredCommandLines; // Stores line number of filtered command
		}					CommandCallbackData;

		EditorConsole();

		void ClearLog();
		void ClearCommand();
		void AddLog(int logLevel, const char* fmt, ...) IM_FMTARGS(2);
		void AddCommand(const char* fmt, ...) IM_FMTARGS(2);
		void Draw();

	private:
		void DrawLog();
		void DrawConsoleCommand();
	};

}

namespace spdlog {

	template<typename Mutex>
	class editorlog_sink : public sinks::base_sink <Mutex>
	{
	protected:
		void sink_it_(const details::log_msg& msg) override
		{
			// log_msg is a struct containing the log entry info like level, timestamp, thread id etc.
			// msg.raw contains pre formatted log

			// If needed (very likely but not mandatory), the sink formats the message before sending it to its final destination:
			spdlog::memory_buf_t formatted;
			sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
			ZeoEngine::EditorConsole::s_Instance.AddLog(msg.level, fmt::to_string(formatted).c_str());
		}

		void flush_() override
		{
			
		}
	};

#include "spdlog/details/null_mutex.h"
#include <mutex>
	namespace sinks {
		using editorlog_sink_mt = editorlog_sink<std::mutex>;
		using editorlog_sink_st = editorlog_sink<details::null_mutex>;
	}

}

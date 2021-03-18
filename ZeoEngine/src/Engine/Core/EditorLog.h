#pragma once

#include <imgui.h>
#include <spdlog/sinks/base_sink.h>

namespace ZeoEngine {

	// Helper: Parse and apply text filters. In format "aaaaa[,bbbb][,ccccc]"
	struct TextFilter
	{
		// [Internal]
		struct TextRange
		{
			const char* b;
			const char* e;

			TextRange() { b = e = NULL; }
			TextRange(const char* _b, const char* _e) { b = _b; e = _e; }
			bool            empty() const { return b == e; }
			IMGUI_API void  split(char separator, ImVector<TextRange>* out) const;
		};

		char                    InputBuf[256];
		ImVector<TextRange>		Filters;
		int                     CountGrep;

		IMGUI_API           TextFilter(const char* default_filter = "");
		IMGUI_API bool      Draw(const char* label = "Filter (inc,-exc)", float width = 0.0f);  // Helper calling InputTextWithHint+Build
		IMGUI_API bool      PassFilter(const char* text, const char* text_end = NULL) const;
		IMGUI_API void      Build();
		void                Clear() { InputBuf[0] = 0; Build(); }
		bool                IsActive() const { return !Filters.empty(); }
	};

	struct EditorLog
	{
		static EditorLog s_EditorLog;

		ImGuiTextBuffer     Buffer;
		TextFilter			Filter;
		ImVector<int>       LineOffsets;	// Index to lines offset. We maintain this with AddLog() calls, allowing us to have a random access on lines
		ImVector<int>		LogLevels;
		ImVector<ImVec4>	LogColors;
		bool                bAutoScroll;	// Keep scrolling if already at the bottom
		bool				bEnableLogLevelFilters[6];

		EditorLog();
		void Clear();
		void AddLog(int logLevel, const char* fmt, ...) IM_FMTARGS(2);
		void Draw();
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
			ZeoEngine::EditorLog::s_EditorLog.AddLog(msg.level, fmt::to_string(formatted).c_str());
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

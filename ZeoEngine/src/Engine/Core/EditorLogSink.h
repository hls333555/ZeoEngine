#pragma once

#include <imgui.h>
#include <spdlog/sinks/base_sink.h>

namespace ZeoEngine {

	struct EditorLog
	{
		static EditorLog s_EditorLog;

		ImGuiTextBuffer     Buf;
		ImGuiTextFilter     Filter;
		ImVector<int>       LineOffsets;        // Index to lines offset. We maintain this with AddLog() calls, allowing us to have a random access on lines
		ImVector<ImVec4>	LogColors;
		bool                AutoScroll;     // Keep scrolling if already at the bottom

		EditorLog()
		{
			AutoScroll = true;
			Clear();
		}

		void Clear()
		{
			Buf.clear();
			LineOffsets.clear();
			LineOffsets.push_back(0);
			LogColors.clear();
		}

		void AddLog(int logLevel, const char* fmt, ...) IM_FMTARGS(2)
		{
			int old_size = Buf.size();
			va_list args;
			va_start(args, fmt);
			Buf.appendfv(fmt, args);
			va_end(args);
			for (int new_size = Buf.size(); old_size < new_size; old_size++)
			{
				if (Buf[old_size] == '\n')
				{
					LineOffsets.push_back(old_size + 1);
					switch (logLevel)
					{
						// Trace
					case 0:
						LogColors.push_back(ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
						break;
						// Debug
					case 1:
						LogColors.push_back(ImVec4(0.0f, 1.0f, 1.0f, 1.0f));
						break;
						// Info
					case 2:
						LogColors.push_back(ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
						break;
						// Warn
					case 3:
						LogColors.push_back(ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
						break;
						// Error
					case 4:
						LogColors.push_back(ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
						break;
						// Critical
					case 5:
						LogColors.push_back(ImVec4(1.0f, 0.5f, 0.0f, 1.0f));
						break;
					default:
						break;
					}
				}
			}
		}

		void Draw(const char* title, bool* p_open = NULL)
		{
			if (!ImGui::Begin(title, p_open))
			{
				ImGui::End();
				return;
			}

			// Options menu
			if (ImGui::BeginPopup("Options"))
			{
				ImGui::Checkbox("Auto-scroll", &AutoScroll);
				ImGui::EndPopup();
			}

			// Main window
			if (ImGui::Button("Options"))
				ImGui::OpenPopup("Options");
			ImGui::SameLine();
			bool clear = ImGui::Button("Clear");
			ImGui::SameLine();
			bool copy = ImGui::Button("Copy");
			ImGui::SameLine();
			Filter.Draw("Filter", -100.0f);

			ImGui::Separator();
			ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

			if (clear)
				Clear();
			if (copy)
				ImGui::LogToClipboard();

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
			const char* buf = Buf.begin();
			const char* buf_end = Buf.end();
			if (Filter.IsActive())
			{
				// In this example we don't use the clipper when Filter is enabled.
				// This is because we don't have a random access on the result on our filter.
				// A real application processing logs with ten of thousands of entries may want to store the result of search/filter.
				// especially if the filtering function is not trivial (e.g. reg-exp).
				for (int line_no = 0; line_no < LineOffsets.Size; ++line_no)
				{
					const char* line_start = buf + LineOffsets[line_no];
					const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
					if (Filter.PassFilter(line_start, line_end) && line_no < LogColors.size())
					{
						ImGui::PushStyleColor(ImGuiCol_Text, LogColors[line_no]);
						ImGui::TextUnformatted(line_start, line_end);
						ImGui::PopStyleColor();
					}
				}
			}
			else
			{
				// The simplest and easy way to display the entire buffer:
				//   ImGui::TextUnformatted(buf_begin, buf_end);
				// And it'll just work. TextUnformatted() has specialization for large blob of text and will fast-forward to skip non-visible lines.
				// Here we instead demonstrate using the clipper to only process lines that are within the visible area.
				// If you have tens of thousands of items and their processing cost is non-negligible, coarse clipping them on your side is recommended.
				// Using ImGuiListClipper requires A) random access into your data, and B) items all being the  same height,
				// both of which we can handle since we an array pointing to the beginning of each line of text.
				// When using the filter (in the block of code above) we don't have random access into the data to display anymore, which is why we don't use the clipper.
				// Storing or skimming through the search result would make it possible (and would be recommended if you want to search through tens of thousands of entries)
				ImGuiListClipper clipper;
				clipper.Begin(LineOffsets.Size);
				while (clipper.Step())
				{
					for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; ++line_no)
					{
						const char* line_start = buf + LineOffsets[line_no];
						const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
						if (line_no < LogColors.size())
						{
							ImGui::PushStyleColor(ImGuiCol_Text, LogColors[line_no]);
							ImGui::TextUnformatted(line_start, line_end);
							ImGui::PopStyleColor();
						}
					}
				}
				clipper.End();
			}
			ImGui::PopStyleVar();

			if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
				ImGui::SetScrollHereY(1.0f);

			ImGui::EndChild();
			ImGui::End();
		}
	};

}

namespace spdlog {

	template<typename Mutex>
	class editorlog_sink : public spdlog::sinks::base_sink <Mutex>
	{
	protected:
		void sink_it_(const spdlog::details::log_msg& msg) override
		{
			// log_msg is a struct containing the log entry info like level, timestamp, thread id etc.
			// msg.raw contains pre formatted log

			// If needed (very likely but not mandatory), the sink formats the message before sending it to its final destination:
			spdlog::memory_buf_t formatted;
			base_sink<Mutex>::formatter_->format(msg, formatted);
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
		using editorlog_sink_st = editorlog_sink<spdlog::details::null_mutex>;
	}

}

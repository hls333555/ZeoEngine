#include "ZEpch.h"
#include "Engine/ImGui/EditorConsole.h"

#include <IconsFontAwesome5.h>

#include "Engine/ImGui/MyImGui.h"

namespace ZeoEngine {

	EditorConsole EditorConsole::s_EditorLog;

	EditorConsole::EditorConsole()
	{
		bAutoScroll = true;
		for (int i = 0; i < 6; ++i)
		{
			bEnableLogLevelFilters[i] = true;
		}
		ClearLog();
		ClearCommand();
	}

	void EditorConsole::ClearLog()
	{
		LogBuffer.clear();
		LogLineOffsets.clear();
		LogLineOffsets.push_back(0);
		LogColors.clear();
	}

	void EditorConsole::ClearCommand()
	{
		CommandCallbackData.CommandBuffer.clear();
		CommandCallbackData.CommandLineOffsets.clear();
		CommandCallbackData.CommandLineOffsets.push_back(0);
	}

	void EditorConsole::AddLog(int logLevel, const char* fmt, ...)
	{
		int old_size = LogBuffer.size();
		va_list args;
		va_start(args, fmt);
		LogBuffer.appendfv(fmt, args);
		va_end(args);
		for (int new_size = LogBuffer.size(); old_size < new_size; ++old_size)
		{
			if (LogBuffer[old_size] == '\n')
			{
				LogLineOffsets.push_back(old_size + 1);
				LogLevels.push_back(logLevel);
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

	void EditorConsole::AddCommand(const char* fmt, ...)
	{
		int old_size = CommandCallbackData.CommandBuffer.size();
		va_list args;
		va_start(args, fmt);
		CommandCallbackData.CommandBuffer.appendfv(fmt, args);
		va_end(args);
		for (int new_size = CommandCallbackData.CommandBuffer.size(); old_size < new_size; ++old_size)
		{
			if (CommandCallbackData.CommandBuffer[old_size] == '\n')
			{
				CommandCallbackData.CommandLineOffsets.push_back(old_size + 1);
			}
		}
	}

	void EditorConsole::Draw()
	{
		DrawLog();

		ImGui::Separator();

		DrawConsoleCommand();
	}

	void EditorConsole::DrawLog()
	{
		// Options menu
		if (ImGui::BeginPopupWithPadding("Options"))
		{
			ImGui::Checkbox(ICON_FA_SCROLL "  Auto-scroll", &bAutoScroll);
			ImGui::EndPopup();
		}
		if (ImGui::Button(ICON_FA_COG "  Options"))
		{
			ImGui::OpenPopup("Options");
		}

		ImGui::SameLine();

		bool bWillClearLog = ImGui::Button(ICON_FA_TRASH_ALT "  Clear");

		ImGui::SameLine();

		bool bWillCopy = ImGui::Button(ICON_FA_COPY "  Copy");

		ImGui::SameLine();

		// Filters menu
		if (ImGui::BeginPopupWithPadding("Filters"))
		{
			ImGui::Checkbox("Trace", &bEnableLogLevelFilters[0]);
			ImGui::Checkbox("Info", &bEnableLogLevelFilters[2]);
			ImGui::Checkbox("Warn", &bEnableLogLevelFilters[3]);
			ImGui::Checkbox("Error", &bEnableLogLevelFilters[4]);
			ImGui::Checkbox("Critical", &bEnableLogLevelFilters[5]);
			ImGui::EndPopup();
		}
		if (ImGui::Button(ICON_FA_FILTER  "Filters"))
		{
			ImGui::OpenPopup("Filters");
		}

		ImGui::SameLine();

		LogFilter.Draw("##LogTextFilter", "Search log", -1.0f);

		ImGui::Separator();

		ImGui::BeginChild("LogScrolling", ImVec2(0, ImGui::GetContentRegionAvail().y - ImGui::GetFrameHeight() - ImGui::GetFramePadding().y * 3), false, ImGuiWindowFlags_HorizontalScrollbar);
		{
			if (bWillClearLog)
			{
				ClearLog();
			}

			if (bWillCopy)
			{
				ImGui::LogToClipboard();
			}

			bool bAnyLogLevelFiltered = false;
			for (auto bEnable : bEnableLogLevelFilters)
			{
				if (!bEnable)
				{
					bAnyLogLevelFiltered = true;
					break;
				}
			}

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
			const char* buf = LogBuffer.begin();
			const char* bufEnd = LogBuffer.end();
			if (LogFilter.IsActive() || bAnyLogLevelFiltered)
			{
				// We don't use the clipper when Filter is enabled.
				// This is because we don't have a random access on the result on our filter.
				// A real application processing logs with ten of thousands of entries may want to store the result of search/filter.
				// especially if the filtering function is not trivial (e.g. reg-exp).
				for (int lineNum = 0; lineNum < LogLineOffsets.Size; ++lineNum)
				{
					const char* lineStart = buf + LogLineOffsets[lineNum];
					const char* lineEnd = (lineNum + 1 < LogLineOffsets.Size) ? (buf + LogLineOffsets[lineNum + 1] - 1) : bufEnd;
					if ((LogFilter.IsActive() ? LogFilter.PassFilter(lineStart, lineEnd) : true) && lineNum < LogColors.size() && bEnableLogLevelFilters[LogLevels[lineNum]])
					{
						ImGui::PushStyleColor(ImGuiCol_Text, LogColors[lineNum]);
						ImGui::TextUnformatted(lineStart, lineEnd);
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
				clipper.Begin(LogLineOffsets.Size, ImGui::GetTextLineHeightWithSpacing());
				while (clipper.Step())
				{
					for (int lineNum = clipper.DisplayStart; lineNum < clipper.DisplayEnd; ++lineNum)
					{
						const char* lineStart = buf + LogLineOffsets[lineNum];
						const char* lineEnd = (lineNum + 1 < LogLineOffsets.Size) ? (buf + LogLineOffsets[lineNum + 1] - 1) : bufEnd;
						if (lineNum < LogColors.size() && bEnableLogLevelFilters[LogLevels[lineNum]])
						{
							ImGui::PushStyleColor(ImGuiCol_Text, LogColors[lineNum]);
							ImGui::TextUnformatted(lineStart, lineEnd);
							ImGui::PopStyleColor();
						}
					}
				}
				clipper.End();
			}
			ImGui::PopStyleVar();

			if (bAutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
			{
				ImGui::SetScrollHereY(1.0f);
			}
		}
		ImGui::EndChild();
	}

	static int GetUnFilteredCommandLineNum(const EditorConsole::CommandInputState& state, int commandIndex)
	{
		return state.FilteredCommandLines[commandIndex];
	}

	static void SetCommandFromActiveIndex(ImGuiInputTextCallbackData* data, int commandIndex)
	{
		const auto& state = *static_cast<EditorConsole::CommandInputState*>(data->UserData);

		const char* command = state.CommandBuffer.begin() + state.CommandLineOffsets[GetUnFilteredCommandLineNum(state, commandIndex)];
		// Begin position of next line - begin position of current line - 1('\n)
		const size_t length = state.CommandLineOffsets[GetUnFilteredCommandLineNum(state, commandIndex) + 1] -
			state.CommandLineOffsets[GetUnFilteredCommandLineNum(state, commandIndex)] - 1;

		memmove(data->Buf, command, length);
		data->Buf[length] = '\0';

		data->BufTextLen = (int)length;
		data->CursorPos = (int)length;
		data->BufDirty = true;
	}

	static int CommandInputCallback(ImGuiInputTextCallbackData* data)
	{
		auto& state = *static_cast<EditorConsole::CommandInputState*>(data->UserData);

		switch (data->EventFlag)
		{
		case ImGuiInputTextFlags_CallbackCompletion:

			if (state.bIsPopupOpen && state.ActiveIdx > -1)
			{
				// Tab was pressed, grab the command text
				SetCommandFromActiveIndex(data, state.ActiveIdx);

				// Hide the popup
				state.bIsPopupOpen = false;
				state.ActiveIdx = -1;
				state.ClickedIdx = -1;
				state.bIsAutoCompletion = true;
			}

			break;

		case ImGuiInputTextFlags_CallbackHistory:

			if (!state.bIsPopupOpen) return 0;

			if (data->EventKey == ImGuiKey_UpArrow)
			{
				if (--state.ActiveIdx < 0)
				{
					state.ActiveIdx = state.FilteredCommandLines.Size - 1;
				}
				state.bSelectionChanged = true;
			}
			else if (data->EventKey == ImGuiKey_DownArrow)
			{
				if (++state.ActiveIdx > state.FilteredCommandLines.Size - 1)
				{
					state.ActiveIdx = 0;
				}
				state.bSelectionChanged = true;
			}

			break;

		case ImGuiInputTextFlags_CallbackAlways:

			state.bIsAutoCompletion = false;
			if (state.ClickedIdx > -1)
			{
				// The user has clicked a command, grab the command text
				SetCommandFromActiveIndex(data, state.ClickedIdx);

				// Hide the popup
				state.bIsPopupOpen = false;
				state.ActiveIdx = -1;
				state.ClickedIdx = -1;
				state.bIsAutoCompletion = true;
			}

			break;
		}

		return 0;
	}

	// https://github.com/ocornut/imgui/issues/718
	void EditorConsole::DrawConsoleCommand()
	{
		const char* buf = CommandCallbackData.CommandBuffer.begin();
		const char* bufEnd = CommandCallbackData.CommandBuffer.end();

		CommandFilter.Draw("##CommandTextFilter", "Input Console Command", -1.0f, CommandInputCallback, &CommandCallbackData);
		// Restore focus to the input box if we just clicked an item
		if (CommandCallbackData.ClickedIdx > -1)
		{
			// NOTE: We do not reset the 'clickedIdx' here because
			// we want to let the callback handle it in order to
			// modify the buffer, therefore we simply restore keyboard input instead
			// so that callback will be invoked next frame
			ImGui::SetKeyboardFocusHere(-1);
		}
		if (ImGui::IsItemDeactivatedAfterEdit() && ImGui::IsKeyPressed(ImGuiKey_Enter))
		{
			if (CommandCallbackData.bIsPopupOpen && CommandCallbackData.ActiveIdx != -1)
			{
				// This means that enter was pressed whilst
				// the popup was open and we had an 'active' item
				const char* command = buf + CommandCallbackData.CommandLineOffsets[GetUnFilteredCommandLineNum(CommandCallbackData, CommandCallbackData.ActiveIdx)];
				// Begin position of next line - begin position of current line - 1('\n)
				const size_t length = CommandCallbackData.CommandLineOffsets[GetUnFilteredCommandLineNum(CommandCallbackData, CommandCallbackData.ActiveIdx) + 1] -
					CommandCallbackData.CommandLineOffsets[GetUnFilteredCommandLineNum(CommandCallbackData, CommandCallbackData.ActiveIdx)] - 1;

				memmove(CommandFilter.InputBuf, command, length);
				CommandFilter.InputBuf[length] = '\0';
			}

			// Commit command
			{
				bool bCommandExist = false;
				for (int lineNum = 0; lineNum < CommandCallbackData.CommandLineOffsets.Size; ++lineNum)
				{
					const char* lineStart = buf + CommandCallbackData.CommandLineOffsets[lineNum];
					const char* lineEnd = (lineNum + 1 < CommandCallbackData.CommandLineOffsets.Size) ? (buf + CommandCallbackData.CommandLineOffsets[lineNum + 1] - 1) : bufEnd;
					std::string command(lineStart, lineEnd - lineStart);
					if (command == CommandFilter.InputBuf)
					{
						ZE_CORE_INFO("Executing command: {0}", CommandFilter.InputBuf);
						bCommandExist = true;
						break;
					}
				}
				if (!bCommandExist)
				{
					ZE_CORE_WARN("Unknown command: {0}", CommandFilter.InputBuf);
				}
			}

			// Clear input
			CommandFilter.InputBuf[0] = '\0';
			// Focus to the input box so that commands can be entered continuously
			ImGui::SetKeyboardFocusHere(-1);

			// Hide popup
			CommandCallbackData.bIsPopupOpen = false;
			CommandCallbackData.ActiveIdx = -1;
		}

		// Prefilter when input changes
		if (CommandFilter.bIsInputBufferChanged)
		{
			if (!CommandCallbackData.bIsAutoCompletion)
			{
				CommandCallbackData.bIsPopupOpen = true;
			}

			CommandCallbackData.FilteredCommandLines.clear();
			CommandCallbackData.ActiveIdx = -1;
			MaxCommandLineWidth = 0.0f;
			for (int lineNum = 0; lineNum < CommandCallbackData.CommandLineOffsets.Size; ++lineNum)
			{
				const char* lineStart = buf + CommandCallbackData.CommandLineOffsets[lineNum];
				const char* lineEnd = (lineNum + 1 < CommandCallbackData.CommandLineOffsets.Size) ? (buf + CommandCallbackData.CommandLineOffsets[lineNum + 1] - 1) : bufEnd;
				if (CommandFilter.IsActive() && CommandFilter.PassFilter(lineStart, lineEnd))
				{
					CommandCallbackData.FilteredCommandLines.push_back(lineNum);
					MaxCommandLineWidth = glm::max(MaxCommandLineWidth, ImGui::CalcTextSize(lineStart, lineEnd).x);
				}
			}
		}
		if (!CommandCallbackData.FilteredCommandLines.empty() && (CommandFilter.bIsInputBufferChanged || CommandCallbackData.bIsPopupOpen))
		{
			const float maxWidth = ImGui::GetItemRectMax().x - ImGui::GetItemRectMin().x;
			const float width = glm::min(maxWidth, MaxCommandLineWidth + ImGui::GetFramePadding().x * 6);
			const int maxLine = 10;
			const float height = glm::min(maxLine, CommandCallbackData.FilteredCommandLines.Size) * ImGui::GetFontSize() + ImGui::GetFramePadding().y * 6;
			const ImVec2 commandListSize = { width , height };
			ImGuiWindowFlags flags =
				ImGuiWindowFlags_NoTitleBar |
				ImGuiWindowFlags_NoResize |
				ImGuiWindowFlags_NoMove |
				ImGuiWindowFlags_NoSavedSettings |
				ImGuiWindowFlags_NoFocusOnAppearing;
			ImGui::SetNextWindowPos({ ImGui::GetItemRectMin().x, ImGui::GetItemRectMin().y - commandListSize.y - ImGui::GetFramePadding().y * 2 });
			ImGui::SetNextWindowSize(commandListSize);
			ImGui::Begin("CommandPopup", nullptr, flags);
			{
				ImGui::PushAllowKeyboardFocus(false);
				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
				for (int idx = 0; idx < CommandCallbackData.FilteredCommandLines.size(); ++idx)
				{
					const int lineNum = GetUnFilteredCommandLineNum(CommandCallbackData, idx);
					const char* lineStart = buf + CommandCallbackData.CommandLineOffsets[lineNum];
					const char* lineEnd = (lineNum + 1 < CommandCallbackData.CommandLineOffsets.Size) ? (buf + CommandCallbackData.CommandLineOffsets[lineNum + 1] - 1) : bufEnd;
					if (CommandFilter.IsActive() && CommandFilter.PassFilter(lineStart, lineEnd))
					{
						// Track if we're drawing the active index so we
						// can scroll to it if it has changed
						bool bIsIndexActive = CommandCallbackData.ActiveIdx == idx;

						ImGui::PushID(lineNum);
						{
							std::string command(lineStart, lineEnd - lineStart);
							if (ImGui::Selectable(command.c_str(), bIsIndexActive))
							{
								// And item was clicked, notify the input
								// callback so that it can modify the input buffer
								CommandCallbackData.ClickedIdx = idx;
							}
						}
						ImGui::PopID();

						if (bIsIndexActive)
						{
							if (CommandCallbackData.bSelectionChanged)
							{
								// Make sure we bring the currently 'active' item into view
								ImGui::SetScrollHereY(1);
								CommandCallbackData.bSelectionChanged = false;
							}
						}
					}
				}
				ImGui::PopStyleVar();
				ImGui::PopAllowKeyboardFocus();
			}
			ImGui::End();
		}
	}
}

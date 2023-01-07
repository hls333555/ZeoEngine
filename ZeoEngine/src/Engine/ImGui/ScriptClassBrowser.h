#pragma once

#include <imgui.h>
#include <IconsFontAwesome5.h>

#include "Engine/ImGui/TextFilter.h"
#include "Engine/Scripting/ScriptEngine.h"

namespace ZeoEngine {

	struct ScriptClassBrowser
	{
		TextFilter Filter;
		U32 LastFilteredCount = 0;

		ScriptClassBrowser() = default;

		/**
		 * Draw the script class browsing widget.
		 *
		 * @param outClassName - class name of the selected script
		 * @param rightPadding - How much space to remain after drawing the combobox widget
		 * @return True if the selection changed
		 */
		bool Draw(std::string& outClassName, float rightPadding)
		{
			bool bIsValueChanged = false;

			ImGui::PushID("ScriptClassBrowser");
			{
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - rightPadding);

				static constexpr int maxCount = 10;
				const int classCount = glm::clamp(static_cast<int>(LastFilteredCount), 1, maxCount);
				const std::string curClassName = fmt::format("{} {}", ICON_FA_FILE_CODE, outClassName);
				const float fontSize = ImGui::GetFontSize();
				if (ImGui::BeginComboFilterWithPadding("", outClassName.empty() ? "" : curClassName.c_str(), classCount, fontSize, fontSize + ImGui::GetFramePadding().y * 4/* Clear button and separator */))
				{
					// Clear current selection
					if (ImGui::Selectable("Clear"))
					{
						bIsValueChanged = !outClassName.empty();
						outClassName.clear();
					}

					ImGui::Separator();

					// Auto-focus search box at startup
					// https://github.com/ocornut/imgui/issues/455#issuecomment-167440172
					if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(ImGuiMouseButton_Left))
					{
						ImGui::SetKeyboardFocusHere();
						ImGui::SetScrollHereY();
					}
					Filter.Draw("##ScriptClassFilter", "Search scripts");

					const float spacing = GImGui->Style.ItemSpacing.y;
					const ImVec2 size = { -FLT_MIN, ( fontSize + spacing) * classCount - spacing + ImGui::GetFramePadding().y * 2 };
					if (ImGui::BeginListBox("##ScriptClassBrowserList", size))
					{
						bool bIsListEmpty = true;
						LastFilteredCount = 0;

						// List all script classes
						const auto& entityClasses = ScriptEngine::GetEntityClasses();
						for (const auto& [name, entityClass] : entityClasses)
						{
							const char* nameStr = name.c_str();
							if (!Filter.IsActive() || Filter.IsActive() && Filter.PassFilter(nameStr))
							{
								bIsListEmpty = false;
								++LastFilteredCount;

								// Push class name as ID
								ImGui::PushID(nameStr);
								{
									const std::string displayName = fmt::format("{} {}", ICON_FA_FILE_CODE, nameStr);
									if (ImGui::Selectable(displayName.c_str()))
									{
										bIsValueChanged = name != outClassName;
										outClassName = name;

										ImGui::CloseCurrentPopup();
									}
								}
								ImGui::PopID();
							}
						}

						if (bIsListEmpty)
						{
							Filter.DrawEmptyText();
						}

						ImGui::EndListBox();
					}

					ImGui::EndCombo();
				}
			}
			ImGui::PopID();

			return bIsValueChanged;
		}
	};

}

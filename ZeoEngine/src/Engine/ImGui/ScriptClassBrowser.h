#pragma once

#include <imgui.h>
#include <IconsFontAwesome5.h>

#include "Engine/ImGui/TextFilter.h"
#include "Engine/Scripting/ScriptEngine.h"

namespace ZeoEngine {

	struct ScriptClassBrowser
	{
		TextFilter Filter;

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

				const std::string curClassName = fmt::format("{} {}", ICON_FA_FILE_CODE, outClassName);
				if (ImGui::BeginCombo("", outClassName.empty() ? "" : curClassName.c_str(), ImGuiComboFlags_HeightLarge))
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

					bool bIsListEmpty = true;
					// List all entities
					const auto& entityClasses = ScriptEngine::GetEntityClasses();
					for (const auto& [name, entityClass] : entityClasses)
					{
						const char* nameStr = name.c_str();
						if (!Filter.IsActive() || Filter.IsActive() && Filter.PassFilter(nameStr))
						{
							bIsListEmpty = false;

							// Push class name as ID
							ImGui::PushID(nameStr);
							{
								bool bIsSelected = ImGui::Selectable("", false, 0, ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing()));

								ImGui::SameLine();

								// Display entity name
								ImGui::Text("%s %s", ICON_FA_FILE_CODE, nameStr);

								if (bIsSelected)
								{
									bIsValueChanged = name != outClassName;
									outClassName = name;
								}
							}
							ImGui::PopID();
						}
					}

					if (bIsListEmpty)
					{
						Filter.DrawEmptyText();
					}

					ImGui::EndCombo();
				}
			}
			ImGui::PopID();

			return bIsValueChanged;
		}
	};

}

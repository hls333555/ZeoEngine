#pragma once

#include <imgui.h>
#include <IconsFontAwesome5.h>

#include "Engine/ImGui/TextFilter.h"
#include "Engine/ImGui/MyImGui.h"
#include "Engine/Utils/SceneUtils.h"

namespace ZeoEngine {

	struct EntityBrowser
	{
		TextFilter Filter;

		EntityBrowser() = default;

		/**
		 * Draw the entity browsing widget.
		 *
		 * @param outEntityID - UUID of the selected entity
		 * @param rightPadding - How much space to remain after drawing the combobox widget
		 * @return True if the selection changed
		 */
		bool Draw(UUID& outEntityID, float rightPadding)
		{
			bool bIsValueChanged = false;

			Scene& scene = SceneUtils::GetActiveGameScene();
			Entity curEntity = scene.GetEntityByUUID(outEntityID);
			
			ImGui::PushID("EntityBrowser");
			{
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - rightPadding);

				const std::string curEntityName = fmt::format("{} {}", ICON_FA_DICE_D6, curEntity.GetName());
				if (ImGui::BeginCombo("", curEntity ? curEntityName.c_str() : "", ImGuiComboFlags_HeightLarge))
				{
					// Clear current selection
					if (ImGui::Selectable("Clear"))
					{
						bIsValueChanged = curEntity;
						curEntity = {};
						outEntityID = 0;
					}

					ImGui::Separator();

					// Auto-focus search box at startup
					// https://github.com/ocornut/imgui/issues/455#issuecomment-167440172
					if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(ImGuiMouseButton_Left))
					{
						ImGui::SetKeyboardFocusHere();
						ImGui::SetScrollHereY();
					}
					Filter.Draw("##EntityFilter", "Search entities");


					if (ImGui::BeginChild("EntityBrowserList", ImVec2(0, 300)))
					{
						bool bIsListEmpty = true;
						// List all entities
						auto coreView = scene.GetComponentView<CoreComponent>();
						for (const auto e : coreView)
						{
							Entity entity{ e, scene.shared_from_this() };
							const char* entityNameStr = entity.GetName().c_str();
							if (!Filter.IsActive() || Filter.IsActive() && Filter.PassFilter(entityNameStr))
							{
								bIsListEmpty = false;

								// Push entity ID
								const auto entityID = entity.GetUUID();
								ImGui::PushID(static_cast<int>(entityID));
								{
									bool bIsSelected = ImGui::Selectable("", false, 0, ImVec2(0.0f, ImGui::GetTextLineHeight() * 2));

									ImGui::SameLine();

									// Align text
									ImGui::BeginGroup();
									{
										// Make two lines of text more compact
										ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0.0f, 0.0f });
										// Display entity name
										ImGui::Text("%s %s", ICON_FA_DICE_D6, entityNameStr);
										// Display entity ID
										ImGui::TextColored({ 0.6f, 0.6f, 0.6f, 1.0f }, "UUID: %llu", entityID);
										ImGui::PopStyleVar();
									}
									ImGui::EndGroup();

									if (bIsSelected)
									{
										bIsValueChanged = entity != curEntity;
										curEntity = entity;
										outEntityID = entityID;
									}
								}
								ImGui::PopID();
							}
						}

						if (bIsListEmpty)
						{
							Filter.DrawEmptyText();
						}
					}

					ImGui::EndChild();

					ImGui::EndCombo();
				}

				// Accept entity dragging from the Level Outline
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::MyAcceptDragDropPayload("LevelOutlineEntity"))
					{
						const Entity entity = *static_cast<Entity*>(payload->Data);
						bIsValueChanged = entity != curEntity;
						curEntity = entity;
						outEntityID = entity.GetUUID();
					}
				}
			}
			ImGui::PopID();

			// Display entity ID for current selection
			if (curEntity && ImGui::IsItemHovered())
			{
				ImGui::SetTooltipWithPadding("UUID: %llu", curEntity.GetUUID());
			}

			return bIsValueChanged;
		}
	};

}
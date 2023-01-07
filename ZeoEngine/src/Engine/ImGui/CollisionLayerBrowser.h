#pragma once

#include <imgui.h>
#include <IconsFontAwesome5.h>

#include "Engine/ImGui/TextFilter.h"
#include "Engine/Physics/CollisionLayer.h"

namespace ZeoEngine {

	struct CollisionLayerBrowser
	{
		TextFilter Filter;
		U32 LastFilteredCount = 0;

		CollisionLayerBrowser() = default;

		/**
		 * Draw the collision layer browsing widget.
		 *
		 * @param outCollisionLayer - selected collision layer
		 * @param rightPadding - How much space to remain after drawing the combobox widget
		 * @return True if the selection changed
		 */
		bool Draw(U32& outCollisionLayer, float rightPadding)
		{
			bool bIsValueChanged = false;

			ImGui::PushID("CollisionLayerBrowser");
			{
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - rightPadding);

				static constexpr int maxCount = 10;
				const int layerCount = glm::clamp(static_cast<int>(LastFilteredCount), 1, maxCount);
				const std::string curLayerName = fmt::format("{} {}", ICON_FA_LAYER_GROUP, CollisionLayerManager::GetLayerName(outCollisionLayer));
				if (ImGui::BeginComboFilterWithPadding("", curLayerName.c_str(), layerCount, ImGui::GetFontSize()))
				{
					// Auto-focus search box at startup
					// https://github.com/ocornut/imgui/issues/455#issuecomment-167440172
					if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(ImGuiMouseButton_Left))
					{
						ImGui::SetKeyboardFocusHere();
						ImGui::SetScrollHereY();
					}
					Filter.Draw("##CollisionLayerFilter", "Search collision layers");

					const ImVec2 size = { -FLT_MIN, ImGui::GetTextLineHeightWithSpacing() * layerCount - GImGui->Style.ItemSpacing.y + ImGui::GetFramePadding().y * 2 };
					if (ImGui::BeginListBox("##CollisionLayerBrowserList", size))
					{
						bool bIsListEmpty = true;
						LastFilteredCount = 0;

						// List all collision layers
						const auto& layers = CollisionLayerManager::GetLayers();
						for (const auto& layer : layers)
						{
							if (!Filter.IsActive() || Filter.IsActive() && Filter.PassFilter(layer.Name.c_str()))
							{
								bIsListEmpty = false;
								++LastFilteredCount;

								// Push layer ID
								ImGui::PushID(static_cast<int>(layer.LayerID));
								{
									const std::string name = fmt::format("{} {}", ICON_FA_LAYER_GROUP, layer.Name.c_str());
									if (ImGui::Selectable(name.c_str()))
									{
										bIsValueChanged = layer.LayerID != outCollisionLayer;
										outCollisionLayer = layer.LayerID;

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

	struct CollisionGroupBrowser
	{
		TextFilter Filter;
		U32 LastFilteredCount = 0;

		CollisionGroupBrowser() = default;

		/**
		 * Draw the collision group browsing widget.
		 *
		 * @param outCollisionGroup - selected collision group
		 * @param rightPadding - How much space to remain after drawing the combobox widget
		 * @return True if the selection changed
		 */
		bool Draw(U32& outCollisionGroup, float rightPadding)
		{
			bool bIsValueChanged = false;

			ImGui::PushID("CollisionGroupBrowser");
			{
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - rightPadding);

				static constexpr int maxCount = 10;
				const int groupCount = glm::clamp(static_cast<int>(LastFilteredCount), 1, maxCount);
				const std::string curGroupName = fmt::format("{} {}", ICON_FA_OBJECT_GROUP, CollisionLayerManager::GetGroupName(outCollisionGroup));
				if (ImGui::BeginComboFilterWithPadding("", curGroupName.c_str(), groupCount, ImGui::GetFontSize()))
				{
					// Auto-focus search box at startup
					// https://github.com/ocornut/imgui/issues/455#issuecomment-167440172
					if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(ImGuiMouseButton_Left))
					{
						ImGui::SetKeyboardFocusHere();
						ImGui::SetScrollHereY();
					}
					Filter.Draw("##CollisionGroupFilter", "Search collision groups");

					const ImVec2 size = { -FLT_MIN, ImGui::GetTextLineHeightWithSpacing() * groupCount - GImGui->Style.ItemSpacing.y + ImGui::GetFramePadding().y * 2 };
					if (ImGui::BeginListBox("##CollisionGroupBrowserList", size))
					{
						bool bIsListEmpty = true;
						LastFilteredCount = 0;

						// List all collision groups
						const auto& groups = CollisionLayerManager::GetGroups();
						for (const auto& group : groups)
						{
							if (!Filter.IsActive() || Filter.IsActive() && Filter.PassFilter(group.Name.c_str()))
							{
								bIsListEmpty = false;
								++LastFilteredCount;

								// Push group ID
								ImGui::PushID(static_cast<int>(group.GroupID));
								{
									const std::string name = fmt::format("{} {}", ICON_FA_OBJECT_GROUP, group.Name.c_str());
									if (ImGui::Selectable(name.c_str()))
									{
										bIsValueChanged = group.GroupID != outCollisionGroup;
										outCollisionGroup = group.GroupID;

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

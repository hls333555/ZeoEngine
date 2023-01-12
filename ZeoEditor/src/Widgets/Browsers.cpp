#include <IconsFontAwesome5.h>

#include "Engine/Core/EngineTypes.h"
#include "Engine/GameFramework/Components.h"
#include "Engine/GameFramework/Entity.h"
#include "Widgets/CollisionLayerBrowser.h"
#include "Engine/Physics/CollisionLayer.h"
#include "Engine/ImGui/MyImGui.h"
#include "Engine/Scripting/ScriptEngine.h"
#include "Widgets/EntityBrowser.h"
#include "Engine/Utils/SceneUtils.h"
#include "Widgets/ScriptClassBrowser.h"

namespace ZeoEngine {

	bool CollisionLayerBrowser::Draw(U32& outCollisionLayer, float rightPadding)
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

	bool CollisionGroupBrowser::Draw(U32& outCollisionGroup, float rightPadding)
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

	bool EntityBrowser::Draw(UUID& outEntityID, float rightPadding)
	{
		bool bIsValueChanged = false;

		Scene& scene = SceneUtils::GetActiveGameScene();
		Entity curEntity = scene.GetEntityByUUID(outEntityID);

		ImGui::PushID("EntityBrowser");
		{
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - rightPadding);

			static constexpr int maxCount = 10;
			const int entityCount = glm::clamp(static_cast<int>(LastFilteredCount), 1, maxCount);
			const std::string curEntityName = curEntity ? fmt::format("{} {}", ICON_FA_DICE_D6, curEntity.GetName()) : "";
			const float fontSize = ImGui::GetFontSize();
			const float itemHeight = LastFilteredCount > 0 ? fontSize * 2 : fontSize;
			if (ImGui::BeginComboFilterWithPadding("", curEntityName.c_str(), entityCount, itemHeight, fontSize + ImGui::GetFramePadding().y * 4/* Clear button and separator */))
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

				const float spacing = GImGui->Style.ItemSpacing.y;
				const ImVec2 size = { -FLT_MIN, (itemHeight + spacing) * entityCount - spacing + ImGui::GetFramePadding().y * 2 };
				if (ImGui::BeginListBox("##EntityBrowserList", size))
				{
					bool bIsListEmpty = true;
					LastFilteredCount = 0;
					const auto sceneRef = scene.shared_from_this();

					// List all entities
					auto coreView = scene.GetComponentView<CoreComponent>();
					for (const auto e : coreView)
					{
						Entity entity{ e, sceneRef };
						if (!Filter.IsActive() || Filter.IsActive() && Filter.PassFilter(entity.GetName().c_str()))
						{
							bIsListEmpty = false;
							++LastFilteredCount;

							// Push entity ID
							const auto entityID = entity.GetUUID();
							ImGui::PushID(static_cast<int>(entityID));
							{
								bool bIsSelected = ImGui::Selectable("", false, 0, ImVec2(0.0f, itemHeight));

								ImGui::SameLine();

								// Align text
								ImGui::BeginGroup();
								{
									// Make two lines of text more compact
									ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0.0f, 0.0f });
									// Display entity name
									ImGui::Text("%s %s", ICON_FA_DICE_D6, entity.GetName().c_str());
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

	bool ScriptClassBrowser::Draw(std::string& outClassName, float rightPadding)
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
				const ImVec2 size = { -FLT_MIN, (fontSize + spacing) * classCount - spacing + ImGui::GetFramePadding().y * 2 };
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
	
}

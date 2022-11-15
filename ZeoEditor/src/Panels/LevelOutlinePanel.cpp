#include "Panels/LevelOutlinePanel.h"

#include <imgui.h>
#include <IconsFontAwesome5.h>

#include "Engine/GameFramework/Components.h"
#include "Engine/Core/KeyCodes.h"
#include "Utils/EditorSceneUtils.h"
#include "Worlds/LevelPreviewWorld.h"
#include "Core/Editor.h"

namespace ZeoEngine {

	LevelOutlinePanel::LevelOutlinePanel(std::string panelName)
		: PanelBase(std::move(panelName))
		, m_EditorWorld(g_Editor->GetLevelWorld())
	{
	}

	void LevelOutlinePanel::ProcessRender()
	{
		m_Filter.Draw("##SceneOutlineAssetFilter", "Search entities");
		
		ImGui::Separator();

		const auto sceneRef = m_EditorWorld->GetActiveScene();
		auto& scene = *sceneRef;

		const auto availRegion = ImGui::GetContentRegionAvail();
		const ImVec2 entityListSize = { availRegion.x, availRegion.y - ImGui::GetTextLineHeightWithSpacing() - ImGui::GetFramePadding().y * 2 /* separator */ };
		if (ImGui::BeginChild("SceneOutlineEntityList", entityListSize))
		{
			// Display entities in creation order, the order is updated when a new entity is created or destroyed
			scene.ForEachComponentView<CoreComponent>([this, &sceneRef](auto entityID, auto& coreComp)
			{
				const Entity entity{ entityID, sceneRef };
				DrawEntityNode(entity);
			});

			// Deselect entity when blank space is clicked
			// NOTE: We cannot use IsPanelHovered() here or entities can never be selected on mouse click
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsWindowHovered())
			{
				m_EditorWorld->SetContextEntity({});
			}

			// Right-click on blank space
			if (ImGui::BeginPopupContextWindowWithPadding(nullptr, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
			{
				Entity newEntity;

				if (ImGui::MenuItem(ICON_FA_BULLSEYE "  Create Empty Entity"))
				{
					newEntity = EditorSceneUtils::CreateAndPlaceEntity(scene);
				}

				if (ImGui::BeginMenu(ICON_FA_LIGHTBULB "  Shapes"))
				{
					if (ImGui::MenuItem(ICON_FA_CUBE "  Cube"))
					{
						newEntity = EditorSceneUtils::CreateAndPlaceCube(scene);
					}

					if (ImGui::MenuItem(ICON_FA_CIRCLE "  Sphere"))
					{
						newEntity = EditorSceneUtils::CreateAndPlaceSphere(scene);
					}

					if (ImGui::MenuItem(ICON_FA_SQUARE "  Plane"))
					{
						newEntity = EditorSceneUtils::CreateAndPlacePlane(scene);
					}

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu(ICON_FA_LIGHTBULB "  Lights"))
				{
					if (ImGui::MenuItem(ICON_FA_SUN "  Directional Light"))
					{
						newEntity = EditorSceneUtils::CreateAndPlaceDirectionalLight(scene);
					}

					if (ImGui::MenuItem(ICON_FA_LIGHTBULB "  Point Light"))
					{
						newEntity = EditorSceneUtils::CreateAndPlacePointLight(scene);
					}

					if (ImGui::MenuItem(ICON_FA_LIGHTBULB "  Spot Light"))
					{
						newEntity = EditorSceneUtils::CreateAndPlaceSpotLight(scene);
					}

					ImGui::EndMenu();
				}

				if (newEntity)
				{
					m_EditorWorld->SetContextEntity(newEntity);
				}

				ImGui::EndPopup();
			}
		}
		ImGui::EndChild();

		ImGui::Separator();

		ImGui::Text(" %d entitie(s)", scene.GetEntityCount());
	}

	void LevelOutlinePanel::DrawEntityNode(Entity entity) const
	{
		std::string entityNameStr = entity.GetName();
		const char* entityName = entityNameStr.c_str();
		if (!m_Filter.IsActive() || m_Filter.IsActive() && m_Filter.PassFilter(entityName))
		{
			auto selectedEntity = m_EditorWorld->GetContextEntity();
			ImGuiTreeNodeFlags flags = (selectedEntity == entity ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
			flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
			bool bIsTreeExpanded = ImGui::TreeNodeEx((void*)(U64)(U32)entity, flags, entityName);
			// Display UUID when hovered
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltipWithPadding("UUID: %llu", entity.GetUUID());
			}
			if (ImGui::IsItemClicked())
			{
				m_EditorWorld->SetContextEntity(entity);
			}
			if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsItemHovered())
			{
				m_EditorWorld->FocusContextEntity();
			}

			bool bIsCurrentEntitySelected = selectedEntity == entity;
			bool bWillDestroyEntity = false;
			// NOTE: We cannot use Input::IsKeyReleased() here as it will return true even if key has not been pressed yet
			if (IsPanelFocused() && bIsCurrentEntitySelected && ImGui::IsKeyReleased(Key::Delete))
			{
				bWillDestroyEntity = true;
			}

			if (bIsTreeExpanded)
			{
				// TODO: Display child entities

				ImGui::TreePop();
			}

			// Defer entity destruction
			if (bWillDestroyEntity)
			{
				m_EditorWorld->GetActiveScene()->DestroyEntity(entity);
				if (bIsCurrentEntitySelected)
				{
					m_EditorWorld->SetContextEntity({});
				}
			}
		}
		
	}

}

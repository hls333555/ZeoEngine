#include "Panels/InspectorPanel.h"

#include "Core/Editor.h"
#include "Worlds/AssetPreviewWorlds.h"
#include "Worlds/LevelPreviewWorld.h"
#include "Panels/AssetViewPanel.h"
#include "Engine/GameFramework/Components.h"
#include "Inspectors/MaterialInspector.h"

namespace ZeoEngine {

	InspectorPanel::InspectorPanel(std::string panelName)
		: PanelBase(std::move(panelName))
		, m_EditorWorld(std::static_pointer_cast<EditorPreviewWorldBase>(g_Editor->GetLevelWorld()))
	{
		SetPadding({ 0.0f, 0.0f });
		m_AssetViewPanel = CreateRef<AssetViewPanel>(ASSET_VIEW);
		m_AssetViewPanel->OnAttach();
	}

	void InspectorPanel::UpdateWorld(const Ref<EditorPreviewWorldBase>& world, bool bIncludeAssetViewPanel)
	{
		m_EditorWorld = world;
		if (bIncludeAssetViewPanel)
		{
			m_AssetViewPanel->UpdateWorld(world);
		}
	}

	void InspectorPanel::ToggleAssetView(bool bShow) const
	{
		m_AssetViewPanel->Toggle(bShow);
	}

	void InspectorPanel::InspectLast()
	{
		const auto& lastData = m_InspectHistory.top();
		if (lastData.AssetPath.empty())
		{
			lastData.World.lock()->SetContextEntity(lastData.LevelSelectedEntity);
			g_Editor->InspectLevelEntity();
		}
		else
		{
			g_Editor->InspectAsset(lastData.AssetPath, std::dynamic_pointer_cast<AssetPreviewWorldBase>(lastData.World.lock()), false, true);
		}
		m_InspectHistory.pop();
	}

	std::string InspectorPanel::GetLastInspectHistoryDisplayName() const
	{
		const auto& lastData = m_InspectHistory.top();
		if (lastData.AssetPath.empty())
		{
			return fmt::format("Entity: {}", lastData.LevelSelectedEntity.GetName());
		}
		return fmt::format("Asset: {}", PathUtils::GetPathName(lastData.AssetPath));
	}

	void InspectorPanel::ProcessUpdate(DeltaTime dt)
	{
		if (const Entity selectedEntity = GetEditorWorld()->GetContextEntity())
		{
			m_AssetViewPanel->OnUpdate(dt);
		}
	}

	void InspectorPanel::ProcessRender()
	{
		const ImGuiID dockspaceID = ImGui::GetID("InspectorDockSpace");
		BuildDockspaceLayout(dockspaceID);
		ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

		const auto editorWorld = GetEditorWorld();
		if (const Entity selectedEntity = editorWorld->GetContextEntity())
		{
			m_AssetViewPanel->OnImGuiRender();
			RenderDetailsPanel(editorWorld, selectedEntity);

			m_bShouldDrawWhenNoEntitySelected = true;
		}
		else if (m_bShouldDrawWhenNoEntitySelected)
		{
			m_bShouldDrawWhenNoEntitySelected = false;

			RenderDetailsPanel(editorWorld, selectedEntity);
		}
	}

	void InspectorPanel::ProcessEvent(Event& e)
	{
		if (const Entity selectedEntity = GetEditorWorld()->GetContextEntity())
		{
			m_AssetViewPanel->OnEvent(e);
		}
	}

	void InspectorPanel::RenderDetailsPanel(const Ref<EditorPreviewWorldBase>& editorWorld, Entity selectedEntity)
	{
		ImGui::Begin(DETAILS, nullptr, ImGuiWindowFlags_NoFocusOnAppearing);
		editorWorld->GetInspector().Draw(selectedEntity);
		ImGui::End();
	}

	void InspectorPanel::BuildDockspaceLayout(ImGuiID dockspaceID)
	{
		if (ImGui::DockBuilderGetNode(dockspaceID) == nullptr || g_Editor->ShouldRebuildDockspaceLayout())
		{
			// Clear out existing layout
			ImGui::DockBuilderRemoveNode(dockspaceID);
			// Add empty node
			ImGui::DockBuilderAddNode(dockspaceID, ImGuiDockNodeFlags_NoTabBar | ImGuiDockNodeFlags_NoDocking); // Flags for Details panel which cannot be docked into
			// Main node should cover entire window
			ImGui::DockBuilderSetNodeSize(dockspaceID, ImGui::GetWindowSize());
			// Build layout
			ImGuiID dockDown;
			ImGuiID dockUp = ImGui::DockBuilderSplitNode(dockspaceID, ImGuiDir_Up, 0.4f, nullptr, &dockDown);
			ImGui::DockBuilderDockWindow("###" ASSET_VIEW, dockUp);
			ImGui::DockBuilderDockWindow(DETAILS, dockDown);
			ImGui::DockBuilderFinish(dockspaceID);
		}
	}

}

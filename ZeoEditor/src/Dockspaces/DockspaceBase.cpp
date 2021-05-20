#include "Dockspaces/DockspaceBase.h"

#include <imgui_internal.h>

#include "Editors/EditorBase.h"
#include "Utils/EditorUtils.h"

namespace ZeoEngine {

	DockspaceBase::DockspaceBase(EditorBase* owningEditor)
		: m_OwningEditor(owningEditor)
	{
	}

	void DockspaceBase::OnUpdate(DeltaTime dt)
	{
		m_PanelManager.OnUpdate(dt);
	}

	void DockspaceBase::OnImGuiRender()
	{
		// Render dockspace
		PreRenderDockspace();
		RenderDockspace();
		// Render panels
		m_PanelManager.OnImGuiRender();
	}

	void DockspaceBase::OnEvent(Event& e)
	{
		m_MenuManager.OnEvent(e);
		m_PanelManager.OnEvent(e);
	}

	void DockspaceBase::PreRenderDockspace()
	{
		ImGuiViewport* mainViewport = ImGui::GetMainViewport();
		ImVec2 centerPos{ mainViewport->Pos.x + mainViewport->Size.x / 2.0f, mainViewport->Pos.y + mainViewport->Size.y / 2.0f };
		ImGui::SetNextWindowPos(centerPos, ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
		ImGui::SetNextWindowSize(m_DockspaceSpec.InitialSize.Data, m_DockspaceSpec.InitialSize.Condition);
	}

	void DockspaceBase::RenderDockspace()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		EditorType editorType = m_OwningEditor->GetEditorType();
		const char* editorName = GetEditorName(editorType);
		ImGui::Begin(editorName, m_OwningEditor->GetShowPtr(), m_DockspaceSpec.WindowFlags);
		ImGui::PopStyleVar();

		m_bIsDockspaceFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows);
		m_bIsDockspaceHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);

		// Render menus (non-main-menus must be rendered winthin window context)
		m_MenuManager.OnImGuiRender(editorType == EditorType::MainEditor);

		ImGuiID dockspaceID = ImGui::GetID(editorName);
		if (ImGui::DockBuilderGetNode(dockspaceID) == nullptr || m_bShouldRebuildDockLayout)
		{
			m_bShouldRebuildDockLayout = false;

			// Clear out existing layout
			ImGui::DockBuilderRemoveNode(dockspaceID);
			// Add empty node
			ImGui::DockBuilderAddNode(dockspaceID, ImGuiDockNodeFlags_DockSpace);
			// Main node should cover entire window
			ImGui::DockBuilderSetNodeSize(dockspaceID, ImGui::GetWindowSize());
			// Build dock layout
			BuildDockWindows(dockspaceID);
			ImGui::DockBuilderFinish(dockspaceID);
		}

		// Should be put at last
		ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

		ImGui::End();
	}

	EditorMenu& DockspaceBase::CreateMenu(const std::string& menuName)
	{
		return m_MenuManager.CreateMenu(menuName, m_OwningEditor);
	}

	void DockspaceBase::CreatePanel(PanelType type)
	{
		m_PanelManager.CreatePanel(type, m_OwningEditor);
	}

	void DockspaceBase::OpenPanel(PanelType type)
	{
		m_PanelManager.OpenPanel(type, m_OwningEditor);
	}

	PanelBase* DockspaceBase::GetPanel(PanelType type)
	{
		return m_PanelManager.GetPanel(type);
	}

}

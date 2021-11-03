#include "EditorUIRenderers/EditorUIRendererBase.h"

#include <imgui_internal.h>

#include "Editors/EditorBase.h"
#include "Panels/EditorViewPanelBase.h"
#include "Menus/EditorMenu.h"

namespace ZeoEngine {

	EditorUIRendererBase::EditorUIRendererBase(const Ref<EditorBase>& contextEditor)
		: m_ContextEditor(contextEditor)
	{
	}

	void EditorUIRendererBase::OnUpdate(DeltaTime dt)
	{
		UpdatePanels(dt);
	}

	void EditorUIRendererBase::OnImGuiRender()
	{
		// Render dockspace
		RenderDockspace();
		// Render panels
		RenderPanels();
	}

	void EditorUIRendererBase::OnEvent(Event& e)
	{
		OnEventMenus(e);
		OnEventPanels(e);
	}

	void EditorUIRendererBase::RenderDockspace()
	{
		const std::string& editorName = m_ContextEditor->GetEditorName();
		ImGuiViewport* mainViewport = ImGui::GetMainViewport();
		bool bIsSceneEditor = editorName == LEVEL_EDITOR;
		if (!bIsSceneEditor)
		{
			ImVec2 centerPos{ mainViewport->Pos.x + mainViewport->Size.x / 2.0f, mainViewport->Pos.y + mainViewport->Size.y / 2.0f };
			ImGui::SetNextWindowPos(centerPos, ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
			ImGui::SetNextWindowSize(m_DockspaceSpec.InitialSize.Data, m_DockspaceSpec.InitialSize.Condition);
		}
		else
		{
			ImGui::SetNextWindowPos(mainViewport->WorkPos);
			ImGui::SetNextWindowSize(mainViewport->WorkSize);
			ImGui::SetNextWindowViewport(mainViewport->ID);
		}

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin(m_ContextEditor->GetEditorName().c_str(), m_ContextEditor->GetShowPtr(), m_DockspaceSpec.WindowFlags);
		ImGui::PopStyleVar();

		m_bIsEditorFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows | ImGuiFocusedFlags_DockHierarchy);
		m_bIsEditorHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows | ImGuiFocusedFlags_DockHierarchy);

		// Render menus
		if (bIsSceneEditor)
		{
			RenderMainMenuBar();
		}
		else
		{
			// Non-main-menus must be rendered winthin the window context
			RenderMenuBar();
		}

		ImGuiID dockspaceID = ImGui::GetID(editorName.c_str());
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

	void EditorUIRendererBase::RenderMainMenuBar()
	{
		if (ImGui::BeginMainMenuBar())
		{
			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

			RenderMenus();

			// Display engine stats at right corner of the main menu bar
			{
				const float statsWidth = ImGui::CalcTextSize("%.f FPS (%.2f ms)").x;
				ImGui::Indent(contentRegionAvailable.x - statsWidth);
				ImGui::Text("%.f FPS (%.2f ms)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);
			}

			ImGui::EndMainMenuBar();
		}
	}

	void EditorUIRendererBase::RenderMenuBar()
	{
		if (ImGui::BeginMenuBar())
		{
			RenderMenus();

			ImGui::EndMenuBar();
		}
	}

	void EditorUIRendererBase::RenderMenus()
	{
		for (const auto& menu : m_Menus)
		{
			menu->OnImGuiRender();
		}
	}

	void EditorUIRendererBase::OnEventMenus(Event& e)
	{
		for (const auto& menu : m_Menus)
		{
			menu->OnEvent(e);
		}
	}

	void EditorUIRendererBase::UpdatePanels(DeltaTime dt)
	{
		for (auto& [type, panel] : m_Panels)
		{
			panel->OnUpdate(dt);
		}
	}

	void EditorUIRendererBase::RenderPanels()
	{
		for (auto& [type, panel] : m_Panels)
		{
			panel->OnImGuiRender();
		}
	}

	void EditorUIRendererBase::OnEventPanels(Event& e)
	{
		for (auto& [type, panel] : m_Panels)
		{
			panel->OnEvent(e);
		}
	}

	EditorMenu& EditorUIRendererBase::CreateMenu(const char* menuName)
	{
		Ref<EditorMenu> menu = CreateRef<EditorMenu>(menuName, m_ContextEditor);
		m_Menus.emplace_back(menu);
		return *menu;
	}

	Ref<EditorViewPanelBase> EditorUIRendererBase::GetViewPanel()
	{
		for (auto& [type, panel] : m_Panels)
		{
			auto viewPanel = std::dynamic_pointer_cast<EditorViewPanelBase>(panel);
			if (viewPanel)
			{
				// There should be only one view panel for each editor
				return viewPanel;
			}
		}

		return {};
	}

}

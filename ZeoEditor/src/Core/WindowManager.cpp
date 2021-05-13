#include "Core/WindowManager.h"

#include <imgui.h>
#include <IconsFontAwesome5.h>
#include <magic_enum.hpp>

#include "Dockspaces/MainDockspace.h"
#include "Dockspaces/ParticleEditorDockspace.h"
#include "Engine/Renderer/Renderer2D.h"
#include "Panels/GameViewportPanel.h"
#include "Panels/SceneOutlinePanel.h"
#include "Panels/InspectorPanels.h"
#include "Panels/ContentBrowserPanel.h"
#include "Panels/ConsolePanel.h"
#include "Panels/StatsPanel.h"
#include "Panels/PreferencesPanel.h"
#include "Panels/AboutPanel.h"
#include "Panels/ParticleViewportPanel.h"
#include "Menus/EditorMenu.h"

namespace ZeoEngine {

	const char* GetDockspaceName(DockspaceType dockspaceType)
	{
		switch (dockspaceType)
		{
			case DockspaceType::MainEditor:		return "Main Editor";
			case DockspaceType::ParticleEditor:	return ICON_FA_FIRE "  Particle Editor";
		}

		const char* typeStr = magic_enum::enum_name(dockspaceType).data();
		ZE_CORE_ASSERT("Failed to get name from dockspace type: {0}!", typeStr);
		return nullptr;
	}

	const char* GetPanelName(PanelType panelType)
	{
		switch (panelType)
		{
			case PanelType::GameView:			return ICON_FA_PLAY_CIRCLE "  Game View";
			case PanelType::SceneOutline:		return ICON_FA_SITEMAP "  Scene Outline";
			case PanelType::EntityInspector:	return ICON_FA_INFO_CIRCLE "  Entity Inspector";
			case PanelType::ContentBrowser:		return ICON_FA_FOLDER "  Content Browser";
			case PanelType::Console:			return ICON_FA_TERMINAL "  Console";
			case PanelType::Stats:				return ICON_FA_CHART_PIE "  Stats";
			case PanelType::Preferences:		return ICON_FA_COGS "  Preferences";
			case PanelType::About:				return ICON_FA_QUESTION_CIRCLE "  About";
			case PanelType::ParticleView:		return ICON_FA_PLAY_CIRCLE "  Particle View";
			case PanelType::ParticleInspector:	return ICON_FA_INFO_CIRCLE "  Particle Inspector";
		}

		const char* typeStr = magic_enum::enum_name(panelType).data();
		ZE_CORE_ASSERT("Failed to get name from panel type: {0}!", typeStr);
		return nullptr;
	}

	DockspaceManager::~DockspaceManager()
	{
		for (auto& [type, dockspace] : m_Dockspaces)
		{
			dockspace->OnDetach();
			delete dockspace;
		}
	}

	DockspaceBase* DockspaceManager::ToggleDockspace(DockspaceType dockspaceType, bool bOpen)
	{
		auto* dockspace = GetDockspace(dockspaceType);
		if (dockspace)
		{
			dockspace->m_bShow = bOpen;
			return dockspace;
		}
		else if (bOpen)
		{
			return CreateDockspace(dockspaceType);
		}
		return nullptr;
	}

	DockspaceBase* DockspaceManager::CreateDockspace(DockspaceType dockspaceType)
	{
		const char* dockspaceName = GetDockspaceName(dockspaceType);
		ZE_CORE_INFO("Creating dockspace: {0}", dockspaceName);
		DockspaceSpec spec;
		spec.Type = dockspaceType;
		switch (dockspaceType)
		{
			case DockspaceType::MainEditor:
			{
				spec.WindowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking;
				spec.WindowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
				return CreateDockspace<MainDockspace>(spec);
			}
			case DockspaceType::ParticleEditor:
			{
				return CreateDockspace<ParticleEditorDockspace>(spec);
			}
		}

		ZE_CORE_ASSERT("Failed to create dockspace with unknown dockspace type: {0}!", dockspaceName);
		return nullptr;
	}

	DockspaceBase* DockspaceManager::GetDockspace(DockspaceType dockspaceType)
	{
		auto result = m_Dockspaces.find(dockspaceType);
		return result == m_Dockspaces.end() ? nullptr : result->second;
	}

	void DockspaceManager::OnUpdate(DeltaTime dt)
	{
		Renderer2D::ResetStats();

		for (auto& [type, dockspace] : m_Dockspaces)
		{
			dockspace->OnUpdate(dt);
		}
	}

	void DockspaceManager::OnImGuiRender()
	{
		for (auto& [type, dockspace] : m_Dockspaces)
		{
			dockspace->OnImGuiRender();
		}
	}

	void DockspaceManager::OnEvent(Event& e)
	{
		for (auto& [type, dockspace] : m_Dockspaces)
		{
			dockspace->OnEvent(e);
		}
	}

	void DockspaceManager::RebuildDockLayout(DockspaceType dockspaceType)
	{
		bool bShouldRebuildAll = dockspaceType == DockspaceType::NONE;
		for (auto& [type, dockspace] : m_Dockspaces)
		{
			if (bShouldRebuildAll || type == dockspaceType)
			{
				dockspace->m_bShouldRebuildDockLayout = true;
				if (!bShouldRebuildAll) break;
			}
		}
	}

	PanelManager::~PanelManager()
	{
		for (auto& [type, panel] : m_Panels)
		{
			panel->OnDetach();
			delete panel;
		}
	}

	PanelBase* PanelManager::TogglePanel(PanelType panelType, DockspaceBase* context, bool bOpen)
	{
		auto* panel = GetPanel(panelType);
		if (panel)
		{
			panel->m_bShow = bOpen;
			return panel;
		}
		else if (bOpen)
		{
			return CreatePanel(panelType, context);
		}
		return nullptr;
	}

	PanelBase* PanelManager::CreatePanel(PanelType panelType, DockspaceBase* context)
	{
		const char* panelName = GetPanelName(panelType);
		ZE_CORE_INFO("Creating panel: {0}", panelName);
		PanelSpec spec;
		spec.Type = panelType;
		switch (panelType)
		{
			case PanelType::GameView:
			{
				return CreatePanel<GameViewportPanel>(spec, context);
			}
			case PanelType::SceneOutline:
			{
				return CreatePanel<SceneOutlinePanel>(spec, context);
			}
			case PanelType::EntityInspector:
			{
				return CreatePanel<EntityInspectorPanel>(spec, context);
			}
			case PanelType::ContentBrowser:
			{
				return CreatePanel<ContentBrowserPanel>(spec, context);
			}
			case PanelType::Console:
			{
				spec.Padding = ImGui::GetStyle().WindowPadding;
				return CreatePanel<ConsolePanel>(spec, context);
			}
			case PanelType::Stats:
			{
				spec.WindowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking;
				spec.Padding = ImGui::GetStyle().WindowPadding;
				spec.InitialSize = { { 300.0f, 300.0f } };
				return CreatePanel<StatsPanel>(spec, context);
			}
			case PanelType::Preferences:
			{
				spec.WindowFlags = ImGuiWindowFlags_NoCollapse;
				spec.Padding = ImGui::GetStyle().WindowPadding;
				return CreatePanel<PreferencesPanel>(spec, context);
			}
			case PanelType::About:
			{
				spec.WindowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize;
				spec.InitialSize = { { 300.0f, 200.0f } };
				return CreatePanel<AboutPanel>(spec, context);
			}
			case PanelType::ParticleView:
			{
				return CreatePanel<ParticleViewportPanel>(spec, context);
			}
			case PanelType::ParticleInspector:
			{
				return CreatePanel<ParticleInspectorPanel>(spec, context);
			}
		}

		ZE_CORE_ASSERT("Failed to create panel with unknown panel type: {0}!", panelName);
		return nullptr;
	}

	PanelBase* PanelManager::GetPanel(PanelType panelType)
	{
		auto result = m_Panels.find(panelType);
		return result == m_Panels.end() ? nullptr : result->second;
	}

	void PanelManager::OnUpdate(DeltaTime dt)
	{
		for (auto& [type, panel] : m_Panels)
		{
			panel->OnUpdate(dt);
		}
	}

	void PanelManager::OnImGuiRender()
	{
		for (auto& [type, panel] : m_Panels)
		{
			panel->OnImGuiRender();
		}
	}

	void PanelManager::OnEvent(Event& e)
	{
		for (auto& [type, panel] : m_Panels)
		{
			panel->OnEvent(e);
		}
	}

	MenuManager::~MenuManager()
	{
		for (auto* menu : m_Menus)
		{
			delete menu;
		}
	}

	EditorMenu& MenuManager::CreateMenu(const std::string& menuName, DockspaceBase* context)
	{
		EditorMenu* menu = new EditorMenu(menuName, context);
		m_Menus.emplace_back(menu);
		return *menu;
	}

	void MenuManager::OnImGuiRender(bool bIsMainMenu)
	{
		if (bIsMainMenu)
		{
			RenderMainMenuBar();
		}
		else
		{
			RenderMenuBar();
		}
	}

	void MenuManager::OnEvent(Event& e)
	{
		for (auto* menu : m_Menus)
		{
			menu->OnEvent(e);
		}
	}

	void MenuManager::RenderMainMenuBar()
	{
		if (ImGui::BeginMainMenuBar())
		{
			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

			RenderMenus();

			// Display engine stats at right corner of main menu bar
			{
				const float statsWidth = ImGui::CalcTextSize("%.f FPS (%.2f ms)").x;
				ImGui::Indent(contentRegionAvailable.x - statsWidth);
				ImGui::Text("%.f FPS (%.2f ms)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);
			}

			ImGui::EndMainMenuBar();
		}
	}

	void MenuManager::RenderMenuBar()
	{
		if (ImGui::BeginMenuBar())
		{
			RenderMenus();

			ImGui::EndMenuBar();
		}
	}

	void MenuManager::RenderMenus()
	{
		for (auto* menu : m_Menus)
		{
			menu->OnImGuiRender();
		}
	}

}

#include "Core/WindowManager.h"

#include <IconsFontAwesome5.h>
#include <magic_enum.hpp>

#include "Dockspaces/MainDockspace.h"
#include "Dockspaces/ParticleEditorDockspace.h"
#include "Engine/Renderer/Renderer2D.h"
#include "Panels/GameViewportPanel.h"
#include "Panels/SceneOutlinePanel.h"
#include "Panels/DataInspectorPanel.h"
#include "Panels/ConsolePanel.h"
#include "Panels/StatsPanel.h"
#include "Panels/PreferencesPanel.h"
#include "Panels/AboutPanel.h"
#include "Panels/ParticleViewportPanel.h"
#include "Panels/DataInspectorPanel.h"
#include "Menus/EditorMenu.h"

namespace ZeoEngine {

	const char* GetDockspaceName(EditorDockspaceType dockspaceType)
	{
		switch (dockspaceType)
		{
			case EditorDockspaceType::Main_Editor:		return "Main Editor";
			case EditorDockspaceType::Particle_Editor:	return ICON_FA_FIRE "  Particle Editor";
		}

		const char* typeStr = magic_enum::enum_name(dockspaceType).data();
		ZE_CORE_ASSERT("Failed to get name from dockspace type: {0}!", typeStr);
		return nullptr;
	}

	const char* GetPanelName(EditorPanelType panelType)
	{
		switch (panelType)
		{
			case EditorPanelType::Game_View:			return ICON_FA_PLAY_CIRCLE "  Game View";
			case EditorPanelType::Scene_Outline:		return ICON_FA_SITEMAP "  Scene Outline";
			case EditorPanelType::Entity_Inspector:		return ICON_FA_INFO_CIRCLE "  Entity Inspector";
			case EditorPanelType::Console:				return ICON_FA_TERMINAL "  Console";
			case EditorPanelType::Stats:				return ICON_FA_CHART_PIE "  Stats";
			case EditorPanelType::Preferences:			return ICON_FA_COGS "  Preferences";
			case EditorPanelType::About:				return ICON_FA_QUESTION_CIRCLE "  About";
			case EditorPanelType::Particle_View:		return ICON_FA_PLAY_CIRCLE "  Particle View";
			case EditorPanelType::Particle_Inspector:	return ICON_FA_INFO_CIRCLE "  Particle Inspector";
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

	EditorDockspace* DockspaceManager::ToggleDockspace(EditorDockspaceType dockspaceType, bool bOpen)
	{
		auto* dockspace = GetDockspace(dockspaceType);
		if (dockspace)
		{
			dockspace->m_bShow = bOpen;
			return dockspace;
		}
		else
		{
			if (bOpen)
			{
				return CreateDockspace(dockspaceType);
			}
		}
		return nullptr;
	}

	EditorDockspace* DockspaceManager::CreateDockspace(EditorDockspaceType dockspaceType)
	{
		const char* dockspaceName = GetDockspaceName(dockspaceType);
		ZE_CORE_INFO("Creating dockspace: {0}", dockspaceName);
		EditorDockspaceSpec spec;
		spec.Type = dockspaceType;
		switch (dockspaceType)
		{
			case EditorDockspaceType::Main_Editor:
			{
				spec.Padding = { 5.0f, 5.0f };
				spec.WindowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking |
					ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
					ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
				return CreateDockspace<MainDockspace>(spec);
			}
			case EditorDockspaceType::Particle_Editor:
			{
				return CreateDockspace<ParticleEditorDockspace>(spec);
			}
		}

		ZE_CORE_ASSERT("Failed to create dockspace with unknown dockspace type: {0}!", dockspaceName);
		return nullptr;
	}

	EditorDockspace* DockspaceManager::GetDockspace(EditorDockspaceType dockspaceType)
	{
		auto result = m_Dockspaces.find(dockspaceType);
		return result == m_Dockspaces.end() ? nullptr : result->second;
	}

	void DockspaceManager::OnUpdate(DeltaTime dt)
	{
		Renderer2D::ResetStats();

		for (auto& [type, dockspace] : m_Dockspaces)
		{
			// Do not update scene if this dockspace is invisible
			if (dockspace->m_bShow)
			{
				dockspace->OnUpdate(dt);
			}
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

	void DockspaceManager::RebuildDockLayout(EditorDockspaceType dockspaceType)
	{
		bool bShouldRebuildAll = dockspaceType == EditorDockspaceType::NONE;
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

	EditorPanel* PanelManager::TogglePanel(EditorPanelType panelType, EditorDockspace* context, bool bOpen)
	{
		auto* panel = GetPanel(panelType);
		if (panel)
		{
			panel->m_bShow = bOpen;
			return panel;
		}
		else
		{
			if (bOpen)
			{
				return CreatePanel(panelType, context);
			}
		}
		return nullptr;
	}

	EditorPanel* PanelManager::CreatePanel(EditorPanelType panelType, EditorDockspace* context)
	{
		const char* panelName = GetPanelName(panelType);
		ZE_CORE_INFO("Creating panel: {0}", panelName);
		EditorPanelSpec spec;
		spec.Type = panelType;
		switch (panelType)
		{
			case EditorPanelType::Game_View:
			{
				return CreatePanel<GameViewportPanel>(spec, context);
			}
			case EditorPanelType::Scene_Outline:
			{
				return CreatePanel<SceneOutlinePanel>(spec, context);
			}
			case EditorPanelType::Entity_Inspector:
			{
				return CreatePanel<EntityInspectorPanel>(spec, context);
			}
			case EditorPanelType::Console:
			{
				return CreatePanel<ConsolePanel>(spec, context);
			}
			case EditorPanelType::Stats:
			{
				spec.WindowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking;
				spec.InitialSize = { { 300.0f, 300.0f } };
				return CreatePanel<StatsPanel>(spec, context);
			}
			case EditorPanelType::Preferences:
			{
				spec.WindowFlags = ImGuiWindowFlags_NoCollapse;
				return CreatePanel<PreferencesPanel>(spec, context);
			}
			case EditorPanelType::About:
			{
				spec.WindowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize;
				spec.InitialSize = { { 300.0f, 200.0f } };
				return CreatePanel<AboutPanel>(spec, context);
			}
			case EditorPanelType::Particle_View:
			{
				return CreatePanel<ParticleViewportPanel>(spec, context);
			}
			case EditorPanelType::Particle_Inspector:
			{
				return CreatePanel<ParticleInspectorPanel>(spec, context);
			}
		}

		ZE_CORE_ASSERT("Failed to create panel with unknown panel type: {0}!", panelName);
		return nullptr;
	}

	EditorPanel* PanelManager::GetPanel(EditorPanelType panelType)
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

	EditorMenu& MenuManager::CreateMenu(const std::string& menuName, EditorDockspace* context)
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

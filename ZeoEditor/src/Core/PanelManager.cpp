#include "Core/PanelManager.h"

#include "Utils/EditorUtils.h"
#include "Panels/GameViewportPanel.h"
#include "Panels/SceneOutlinePanel.h"
#include "Panels/InspectorPanels.h"
#include "Panels/ContentBrowserPanel.h"
#include "Panels/ConsolePanel.h"
#include "Panels/StatsPanel.h"
#include "Panels/PreferencesPanel.h"
#include "Panels/AboutPanel.h"
#include "Panels/ParticleViewportPanel.h"

namespace ZeoEngine {

	PanelManager::~PanelManager()
	{
		for (auto& [type, panel] : m_Panels)
		{
			panel->OnDetach();
			delete panel;
		}
	}

	PanelBase* PanelManager::CreatePanel(PanelType type, EditorBase* owningEditor)
	{
		const char* panelName = EditorUtils::GetPanelName(type);
		ZE_CORE_INFO("Creating panel: {0}", panelName);
		PanelBase* panel = nullptr;
		switch (type)
		{
			case PanelType::GameView:			panel = new GameViewportPanel(type, owningEditor); break;
			case PanelType::SceneOutline:		panel = new SceneOutlinePanel(type, owningEditor); break;
			case PanelType::EntityInspector:	panel = new EntityInspectorPanel(type, owningEditor); break;
			case PanelType::ContentBrowser:		panel = new ContentBrowserPanel(type, owningEditor); break;
			case PanelType::Console:			panel = new ConsolePanel(type, owningEditor); break;
			case PanelType::Stats:				panel = new StatsPanel(type, owningEditor); break;
			case PanelType::Preferences:		panel = new PreferencesPanel(type, owningEditor); break;
			case PanelType::About:				panel = new AboutPanel(type, owningEditor); break;
			case PanelType::ParticleView:		panel = new ParticleViewportPanel(type, owningEditor); break;
			case PanelType::ParticleInspector:	panel = new ParticleInspectorPanel(type, owningEditor); break;
			default:							ZE_CORE_ASSERT("Failed to create panel with unknown panel type: {0}!", panelName); return nullptr;
		}

		panel->OnAttach();
		m_Panels.emplace(type, panel);
		return panel;
	}

	PanelBase* PanelManager::OpenPanel(PanelType type, EditorBase* owningEditor)
	{
		PanelBase* panel = GetPanel(type);
		if (panel)
		{
			panel->Open();
			return panel;
		}

		return CreatePanel(type, owningEditor);
	}

	PanelBase* PanelManager::GetPanel(PanelType type)
	{
		auto result = m_Panels.find(type);
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

}

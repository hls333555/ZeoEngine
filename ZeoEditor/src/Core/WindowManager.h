#pragma once

#include <unordered_map>

#include "Core/EditorTypes.h"
#include "Engine/Events/Event.h"
#include "Engine/Core/DeltaTime.h"

namespace ZeoEngine {

	class DockspaceBase;
	class PanelBase;
	class EditorMenu;

	const char* GetDockspaceName(DockspaceType dockspaceType);
	const char* GetPanelName(PanelType panelType);

	class DockspaceManager
	{
	private:
		DockspaceManager() = default;
		~DockspaceManager();
	public:
		DockspaceManager(const DockspaceManager&) = delete;
		DockspaceManager& operator=(const DockspaceManager&) = delete;

		static DockspaceManager& Get()
		{
			static DockspaceManager instance;
			return instance;
		}

		DockspaceBase* ToggleDockspace(DockspaceType dockspaceType, bool bOpen);

		DockspaceBase* CreateDockspace(DockspaceType dockspaceType);
		template<typename T>
		T* CreateDockspace(const DockspaceSpec& spec)
		{
			T* dockspace = new T(spec);
			m_Dockspaces.emplace(spec.Type, dockspace);
			dockspace->OnAttach();
			return dockspace;
		}

		DockspaceBase* GetDockspace(DockspaceType dockspaceType);

		void OnUpdate(DeltaTime dt);
		void OnImGuiRender();
		void OnEvent(Event& e);

		/** Rebuild layout for all dockspaces if dockspaceType is NONE. */
		void RebuildDockLayout(DockspaceType dockspaceType = DockspaceType::NONE);

	private:
		std::unordered_map<DockspaceType, DockspaceBase*> m_Dockspaces;
	};

	class PanelManager
	{
		friend class DockspaceBase;

	private:
		PanelManager() = default;
		~PanelManager();

	public:
		PanelBase* TogglePanel(PanelType panelType, DockspaceBase* context, bool bOpen);

		PanelBase* CreatePanel(PanelType panelType, DockspaceBase* context);
		template<typename T>
		T* CreatePanel(const PanelSpec& spec, DockspaceBase* context)
		{
			T* panel = new T(spec, context);
			m_Panels.emplace(spec.Type, panel);
			panel->OnAttach();
			return panel;
		}

		PanelBase* GetPanel(PanelType panelType);

		void OnUpdate(DeltaTime dt);
		void OnImGuiRender();
		void OnEvent(Event& e);

	private:
		std::unordered_map<PanelType, PanelBase*> m_Panels;
	};

	class MenuManager
	{
		friend class DockspaceBase;

	private:
		MenuManager() = default;
		~MenuManager();

	public:
		EditorMenu& CreateMenu(const std::string& menuName, DockspaceBase* context);

		void OnImGuiRender(bool bIsMainMenu);
		void OnEvent(Event& e);

	private:
		void RenderMainMenuBar();
		void RenderMenuBar();
		void RenderMenus();

	private:
		std::vector<EditorMenu*> m_Menus;
	};

}

#pragma once

#include <unordered_map>

#include <imgui.h>

#include "Engine/ImGui/MyImGui.h"
#include "Utils/EditorUtils.h"
#include "Engine/Events/Event.h"
#include "Engine/Core/DeltaTime.h"

namespace ZeoEngine {

	struct EditorDockspaceSpec
	{
		EditorDockspaceType Type;
		glm::vec2 Padding{ 0.0f };
		ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_MenuBar;
		ImVec2Data InitialSize{ { 800.0f, 600.0f } };
	};

	struct EditorPanelSpec
	{
		EditorPanelType Type;
		ImGuiWindowFlags WindowFlags = 0;
		ImVec2Data InitialSize{ { 800.0f, 600.0f } };
	};

	class EditorDockspace;
	class EditorPanel;
	class EditorMenu;

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

		EditorDockspace* OpenDockspace(EditorDockspaceType dockspaceType);

		EditorDockspace* CreateDockspace(EditorDockspaceType dockspaceType);
		template<typename T>
		T* CreateDockspace(const EditorDockspaceSpec& spec)
		{
			T* dockspace = new T(spec);
			m_Dockspaces.emplace(spec.Type, dockspace);
			dockspace->OnAttach();
			return dockspace;
		}

		EditorDockspace* GetDockspace(EditorDockspaceType dockspaceType);

		void OnUpdate(DeltaTime dt);
		void OnImGuiRender();
		void OnEvent(Event& e);

		void RebuildDockLayout(EditorDockspaceType dockspaceType = EditorDockspaceType::NONE);

	private:
		std::unordered_map<EditorDockspaceType, EditorDockspace*> m_Dockspaces;
	};

	class PanelManager
	{
	public:
		PanelManager() = default;
		~PanelManager();

		EditorPanel* OpenPanel(EditorPanelType panelType, EditorDockspace* context);

		EditorPanel* CreatePanel(EditorPanelType panelType, EditorDockspace* context);
		template<typename T>
		T* CreatePanel(const EditorPanelSpec& spec, EditorDockspace* context)
		{
			T* panel = new T(spec, context);
			m_Panels.emplace(spec.Type, panel);
			panel->OnAttach();
			return panel;
		}

		EditorPanel* GetPanel(EditorPanelType panelType);

		void OnUpdate(DeltaTime dt);
		void OnImGuiRender();
		void OnEvent(Event& e);

	private:
		std::unordered_map<EditorPanelType, EditorPanel*> m_Panels;
	};

	class MenuManager
	{
	public:
		MenuManager() = default;
		~MenuManager();

		EditorMenu& CreateMenu(const std::string& menuName, EditorDockspace* context);

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

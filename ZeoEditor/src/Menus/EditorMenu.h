#pragma once

#include <string>
#include <vector>

#include "Engine/Events/Event.h"

namespace ZeoEngine {

	class EditorMenuItem;
	class EditorDockspace;

	class EditorMenu
	{
	protected:
		EditorMenu() = default;
	public:
		explicit EditorMenu(const std::string& menuName);
		~EditorMenu();

		void OnImGuiRender();
		void OnEvent(Event& e);

		template<typename T = EditorDockspace>
		T* GetContext() { return dynamic_cast<T*>(m_Context); }
		void SetContext(EditorDockspace* contextDockspace) { m_Context = contextDockspace; }
		void SetEnabled(bool bEnabled) { m_bEnabled = bEnabled; }

		void PushMenuItem(EditorMenuItem* menuItem);

	private:
		void RenderMenuItems();

	private:
		std::string m_MenuName;
		EditorDockspace* m_Context;
		bool m_bEnabled{ true };
		std::vector<EditorMenuItem*> m_MenuItems;
	};

	class MenuManager
	{
	public:
		MenuManager() = default;
		~MenuManager();

		void PushMenu(EditorMenu* menu);

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

#pragma once

#include <string>
#include <vector>

namespace ZeoEngine {

	class EditorMenuItem;

	class EditorMenu
	{
	protected:
		EditorMenu() = default;
	public:
		explicit EditorMenu(const std::string& menuName);
		~EditorMenu();

		void OnImGuiRender();

		void SetEnabled(bool bEnabled) { m_bEnabled = bEnabled; }

		void PushMenuItem(EditorMenuItem* menuItem);

	private:
		void RenderMenuItems();

	private:
		std::string m_MenuName;
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

	private:
		void RenderMainMenuBar();
		void RenderMenuBar();
		void RenderMenus();

	private:
		std::vector<EditorMenu*> m_Menus;
	};

}

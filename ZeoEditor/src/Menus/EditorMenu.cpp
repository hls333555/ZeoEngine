#include "Menus/EditorMenu.h"

#include <imgui.h>

#include "Menus/EditorMenuItem.h"

namespace ZeoEngine {

	EditorMenu::EditorMenu(const std::string& menuName)
		: m_MenuName(menuName)
	{
	}

	EditorMenu::~EditorMenu()
	{
		for (auto* menuItem : m_MenuItems)
		{
			delete menuItem;
		}
	}

	void EditorMenu::OnImGuiRender()
	{
		if (ImGui::BeginMenu(m_MenuName.c_str(), m_bEnabled))
		{
			RenderMenuItems();

			ImGui::EndMenu();
		}
	}

	void EditorMenu::PushMenuItem(EditorMenuItem* menuItem)
	{
		m_MenuItems.emplace_back(menuItem);
	}

	void EditorMenu::RenderMenuItems()
	{
		for (auto* menuItem : m_MenuItems)
		{
			menuItem->OnImGuiRender();
		}
	}

	MenuManager::~MenuManager()
	{
		for (auto* menu : m_Menus)
		{
			delete menu;
		}
	}

	void MenuManager::PushMenu(EditorMenu* menu)
	{
		m_Menus.emplace_back(menu);
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

	void MenuManager::RenderMainMenuBar()
	{
		if (ImGui::BeginMainMenuBar())
		{
			RenderMenus();

			// Display engine stats at right corner of main menu bar
			{
				const float statsWidth = 130.0f;
				ImGui::Indent(ImGui::GetWindowSize().x - statsWidth);
				ImGui::Text("%.f FPS (%.2f ms)", ImGui::GetIO().Framerate, 1000.f / ImGui::GetIO().Framerate);
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

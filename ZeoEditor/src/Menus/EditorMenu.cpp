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

	void EditorMenu::OnEvent(Event& e)
	{
		for (auto* menuItem : m_MenuItems)
		{
			menuItem->OnEvent(e);
		}
	}

	void EditorMenu::PushMenuItem(EditorMenuItem* menuItem)
	{
		menuItem->SetContext(this);
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
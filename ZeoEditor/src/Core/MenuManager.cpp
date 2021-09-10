#include "Core/MenuManager.h"

#include <imgui.h>

#include "Menus/EditorMenu.h"

namespace ZeoEngine {

	MenuManager::~MenuManager()
	{
		for (auto* menu : m_Menus)
		{
			delete menu;
		}
	}

	EditorMenu& MenuManager::CreateMenu(const std::string& menuName, EditorBase* owningEditor)
	{
		EditorMenu* menu = new EditorMenu(menuName, owningEditor);
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

#include "Menus/EditorMenu.h"

#include <imgui.h>

#include "Menus/EditorMenuItems.h"

namespace ZeoEngine {

	EditorMenu::EditorMenu(std::string menuName)
		: m_MenuName(std::move(menuName))
	{
	}

	void EditorMenu::OnImGuiRender() const
	{
		if (ImGui::BeginMenu(m_MenuName.c_str(), m_bEnabled))
		{
			RenderMenuItems();

			ImGui::EndMenu();
		}
	}  

	void EditorMenu::OnEvent(Event& e) const
	{
		for (const auto& menuItem : m_MenuItems)
		{
			menuItem->OnEvent(e);
		}
	}

	void EditorMenu::RenderMenuItems() const
	{
		for (const auto& menuItem : m_MenuItems)
		{
			menuItem->OnImGuiRender();
		}
	}

}

#include "Menus/EditorMenu.h"

#include <imgui.h>

#include "Menus/EditorMenuItems.h"

namespace ZeoEngine {

	EditorMenu::EditorMenu(std::string menuName, const Weak<EditorBase>& contextEditor)
		: m_MenuName(std::move(menuName))
		, m_ContextEditor(contextEditor)
	{
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
		for (const auto& menuItem : m_MenuItems)
		{
			menuItem->OnEvent(e);
		}
	}

	void EditorMenu::RenderMenuItems()
	{
		for (const auto& menuItem : m_MenuItems)
		{
			menuItem->OnImGuiRender();
		}
	}

}

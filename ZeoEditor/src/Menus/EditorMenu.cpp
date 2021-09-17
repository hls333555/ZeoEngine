#include "Menus/EditorMenu.h"

#include <imgui.h>

#include "Menus/EditorMenuItems.h"

namespace ZeoEngine {

	EditorMenu::EditorMenu(const std::string& menuName, const Ref<EditorBase>& contextEditor)
		: m_MenuName(menuName)
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

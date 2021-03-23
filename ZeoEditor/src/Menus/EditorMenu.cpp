#include "Menus/EditorMenu.h"

#include <imgui.h>

#include "Menus/EditorMenuItems.h"

namespace ZeoEngine {

	EditorMenu::EditorMenu(const std::string& menuName, DockspaceBase* context)
		: m_MenuName(menuName)
		, m_Context(context)
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

	void EditorMenu::RenderMenuItems()
	{
		for (auto* menuItem : m_MenuItems)
		{
			menuItem->OnImGuiRender();
		}
	}

}

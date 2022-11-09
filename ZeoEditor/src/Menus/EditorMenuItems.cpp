#include "Menus/EditorMenuItems.h"

#include <imgui.h>

#include "Engine/Core/Input.h"
#include "Engine/Asset/AssetRegistry.h"

namespace ZeoEngine {

	MenuItemBase::MenuItemBase(std::string menuItemName, std::string shortcutName)
		: m_MenuItemName(std::move(menuItemName)), m_ShortcutName(std::move(shortcutName))
	{
	}

	void MenuItemBase::OnImGuiRender()
	{
		if (ImGui::MenuItem(m_MenuItemName.c_str(), m_ShortcutName.c_str(), m_bSelected, m_bEnabled))
		{
			OnMenuItemActivated();
		}
	}

	void MenuItemBase::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>(ZE_BIND_EVENT_FUNC(MenuItemBase::OnKeyPressed));
	}

	bool MenuItemBase::OnKeyPressed(KeyPressedEvent& e)
	{
		if (m_ShortcutName.empty() || e.GetRepeatCount() > 0) return false;

		return OnKeyPressedImpl(e);
	}

	MenuItem_Separator::MenuItem_Separator(std::string menuItemName)
		: MenuItemBase(std::move(menuItemName))
	{
	}

	void MenuItem_Separator::OnImGuiRender()
	{
		ImGui::Separator();
	}

	void MenuItem_TogglePanelBase::OnImGuiRender()
	{
		if (!m_bSelected)
		{
			if (auto* panel = g_Editor->GetPanel(m_MenuItemName))
			{
				m_bSelected = panel->GetShowPtr();
			}
		}

		MenuItemBase::OnImGuiRender();
	}

	bool MenuItem_NewLevel::OnKeyPressedImpl(KeyPressedEvent& e)
	{
		if (m_ShortcutName != "CTRL+N") return false;

		const bool bIsCtrlPressed = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		if (e.GetKeyCode() == Key::N && bIsCtrlPressed)
		{
			g_Editor->NewLevel();
			return true;
		}

		return false;
	}

	void MenuItem_NewLevel::OnMenuItemActivated()
	{
		g_Editor->NewLevel();
	}

	bool MenuItem_LoadLevel::OnKeyPressedImpl(KeyPressedEvent& e)
	{
		if (m_ShortcutName != "CTRL+O") return false;

		const bool bIsCtrlPressed = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		if (e.GetKeyCode() == Key::O && bIsCtrlPressed)
		{
			g_Editor->LoadLevel();
			return true;
		}

		return false;
	}

	void MenuItem_LoadLevel::OnMenuItemActivated()
	{
		g_Editor->LoadLevel();
	}

	bool MenuItem_SaveLevel::OnKeyPressedImpl(KeyPressedEvent& e)
	{
		if (m_ShortcutName != "CTRL+S") return false;

		const bool bIsCtrlPressed = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		const bool bIsAltPressed = Input::IsKeyPressed(Key::LeftAlt) || Input::IsKeyPressed(Key::RightAlt);
		if (e.GetKeyCode() == Key::S && bIsCtrlPressed && !bIsAltPressed)
		{
			g_Editor->SaveLevel();
			return true;
		}

		return false;
	}

	void MenuItem_SaveLevel::OnMenuItemActivated()
	{
		g_Editor->SaveLevel();
	}

	bool MenuItem_SaveLevelAs::OnKeyPressedImpl(KeyPressedEvent& e)
	{
		if (m_ShortcutName != "CTRL+ALT+S") return false;

		const bool bIsCtrlPressed = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		const bool bIsAltPressed = Input::IsKeyPressed(Key::LeftAlt) || Input::IsKeyPressed(Key::RightAlt);
		if (e.GetKeyCode() == Key::S && bIsCtrlPressed && bIsAltPressed)
		{
			g_Editor->SaveLevelAs();
			return true;
		}

		return false;
	}

	void MenuItem_SaveLevelAs::OnMenuItemActivated()
	{
		g_Editor->SaveLevelAs();
	}

	void MenuItem_Undo::OnMenuItemActivated()
	{

	}

	void MenuItem_Redo::OnMenuItemActivated()
	{

	}

	void MenuItem_Copy::OnMenuItemActivated()
	{

	}

	void MenuItem_Paste::OnMenuItemActivated()
	{

	}

	void MenuItem_Cut::OnMenuItemActivated()
	{

	}

	void MenuItem_ResetLayout::OnMenuItemActivated()
	{
		g_Editor->RebuildDockspaceLayout();
	}

}

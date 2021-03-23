#include "Menus/EditorMenuItems.h"

#include <imgui.h>

#include "Dockspaces/DockspaceBase.h"
#include "Engine/GameFramework/Scene.h"
#include "Core/WindowManager.h"
#include "Engine/Core/Input.h"
#include "Panels/SceneViewportPanel.h"

namespace ZeoEngine {

	MenuItemBase::MenuItemBase(DockspaceBase* context, const std::string& menuItemName, const std::string& shortcutName)
		: m_Context(context)
		, m_MenuItemName(menuItemName), m_ShortcutName(shortcutName)
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

	MenuItem_Seperator::MenuItem_Seperator(DockspaceBase* context, const std::string& menuItemName)
		: MenuItemBase(context, menuItemName)
	{
	}

	void MenuItem_Seperator::OnImGuiRender()
	{
		ImGui::Separator();
	}

	MenuItem_ToggleEditor::MenuItem_ToggleEditor(DockspaceBase* context, DockspaceType dockspaceType, const std::string& shortcutName)
		: MenuItemBase(context, GetDockspaceName(dockspaceType), shortcutName)
		, m_DockspaceType(dockspaceType)
	{
	}

	void MenuItem_ToggleEditor::OnImGuiRender()
	{
		if (!m_bSelected)
		{
			auto* dockspace = DockspaceManager::Get().GetDockspace(m_DockspaceType);
			if (dockspace)
			{
				m_bSelected = dockspace->GetShowPtr();
			}
		}

		MenuItemBase::OnImGuiRender();
	}

	void MenuItem_ToggleEditor::OnMenuItemActivated()
	{
		if (!m_bSelected)
		{
			DockspaceManager::Get().ToggleDockspace(m_DockspaceType, true);
		}
	}

	MenuItem_TogglePanel::MenuItem_TogglePanel(DockspaceBase* context, PanelType panelType, const std::string& shortcutName)
		: MenuItemBase(context, GetPanelName(panelType), shortcutName)
		, m_PanelType(panelType)
	{
	}

	void MenuItem_TogglePanel::OnImGuiRender()
	{
		if (!m_bSelected)
		{
			auto* panel = m_Context->GetPanel(m_PanelType);
			if (panel)
			{
				m_bSelected = panel->GetShowPtr();
			}
		}

		MenuItemBase::OnImGuiRender();
	}

	void MenuItem_TogglePanel::OnMenuItemActivated()
	{
		if (!m_bSelected)
		{
			m_Context->TogglePanel(m_PanelType, true);
		}
	}

	bool MenuItem_NewScene::OnKeyPressedImpl(KeyPressedEvent& e)
	{
		if (m_ShortcutName != "CTRL+N" || !m_Context->IsDockspaceFocused()) return false;

		bool bIsCtrlPressed = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		switch (e.GetKeyCode())
		{
			case Key::N:
			{
				if (bIsCtrlPressed)
				{
					m_Context->CreateNewScene();
				}
				break;
			}
		}

		return true;
	}

	void MenuItem_NewScene::OnMenuItemActivated()
	{
		m_Context->CreateNewScene();
	}

	bool MenuItem_OpenScene::OnKeyPressedImpl(KeyPressedEvent& e)
	{
		if (m_ShortcutName != "CTRL+O" || !m_Context->IsDockspaceFocused()) return false;

		bool bIsCtrlPressed = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		switch (e.GetKeyCode())
		{
			case Key::O:
			{
				if (bIsCtrlPressed)
				{
					m_Context->OpenScene();
				}
				break;
			}
		}

		return true;
	}

	void MenuItem_OpenScene::OnMenuItemActivated()
	{
		m_Context->OpenScene();
	}

	bool MenuItem_SaveScene::OnKeyPressedImpl(KeyPressedEvent& e)
	{
		if (m_ShortcutName != "CTRL+S" || !m_Context->IsDockspaceFocused()) return false;

		bool bIsCtrlPressed = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		switch (e.GetKeyCode())
		{
			case Key::S:
			{
				if (bIsCtrlPressed)
				{
					m_Context->SaveScene();
				}
				break;
			}
		}

		return true;
	}

	void MenuItem_SaveScene::OnMenuItemActivated()
	{
		m_Context->SaveScene();
	}

	bool MenuItem_SaveSceneAs::OnKeyPressedImpl(KeyPressedEvent& e)
	{
		if (m_ShortcutName != "CTRL+ALT+S" || !m_Context->IsDockspaceFocused()) return false;

		bool bIsCtrlPressed = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		bool bIsAltPressed = Input::IsKeyPressed(Key::LeftAlt) || Input::IsKeyPressed(Key::RightAlt);
		switch (e.GetKeyCode())
		{
			case Key::S:
			{
				if (bIsCtrlPressed)
				{
					if (bIsAltPressed)
					{
						m_Context->SaveSceneAs();
					}
				}
				break;
			}
		}

		return true;
	}

	void MenuItem_SaveSceneAs::OnMenuItemActivated()
	{
		m_Context->SaveSceneAs();
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
		DockspaceManager::Get().RebuildDockLayout();
	}

	void MenuItem_Snapshot::OnMenuItemActivated()
	{
		const std::string filePath = m_Context->GetScene()->GetPath();
		// This may be null e.g. default particle system
		if (filePath.empty()) return;

		SceneViewportPanel* viewportPanel = m_Context->GetPanel<SceneViewportPanel>(m_Context->GetViewportPanelType());
		std::string snapshotName = filePath + ".png";
		viewportPanel->Snapshot(snapshotName, 256);
	}

}

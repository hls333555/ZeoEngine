#include "Menus/EditorMenuItems.h"

#include <imgui.h>

#include "Editors/EditorBase.h"
#include "Engine/GameFramework/Scene.h"
#include "Core/EditorManager.h"
#include "Engine/Core/Input.h"
#include "Panels/SceneViewportPanel.h"
#include "Utils/EditorUtils.h"
#include "Dockspaces/DockspaceBase.h"

namespace ZeoEngine {

	MenuItemBase::MenuItemBase(EditorBase* owningEditor, const std::string& menuItemName, const std::string& shortcutName)
		: m_OwningEditor(owningEditor)
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

	DockspaceBase* MenuItemBase::GetOwningDockspace() const
	{
		return m_OwningEditor->GetDockspace();
	}

	bool MenuItemBase::OnKeyPressed(KeyPressedEvent& e)
	{
		if (m_ShortcutName.empty() || e.GetRepeatCount() > 0) return false;

		return OnKeyPressedImpl(e);
	}

	MenuItem_Seperator::MenuItem_Seperator(EditorBase* owningEditor, const std::string& menuItemName)
		: MenuItemBase(owningEditor, menuItemName)
	{
	}

	void MenuItem_Seperator::OnImGuiRender()
	{
		ImGui::Separator();
	}

	MenuItem_ToggleEditor::MenuItem_ToggleEditor(EditorBase* owningEditor, EditorType editorType, const std::string& shortcutName)
		: MenuItemBase(owningEditor, GetEditorName(editorType), shortcutName)
		, m_EditorType(editorType)
	{
	}

	void MenuItem_ToggleEditor::OnImGuiRender()
	{
		if (!m_bSelected)
		{
			if (auto editor = EditorManager::Get().GetEditor(m_EditorType))
			{
				m_bSelected = editor->GetShowPtr();
			}
		}

		MenuItemBase::OnImGuiRender();
	}

	void MenuItem_ToggleEditor::OnMenuItemActivated()
	{
		if (!m_bSelected)
		{
			EditorManager::Get().OpenEditor(m_EditorType);
		}
	}

	MenuItem_TogglePanel::MenuItem_TogglePanel(EditorBase* owningEditor, PanelType panelType, const std::string& shortcutName)
		: MenuItemBase(owningEditor, GetPanelName(panelType), shortcutName)
		, m_PanelType(panelType)
	{
	}

	void MenuItem_TogglePanel::OnImGuiRender()
	{
		if (!m_bSelected)
		{
			if (auto panel = GetOwningDockspace()->GetPanel(m_PanelType))
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
			GetOwningDockspace()->OpenPanel(m_PanelType);
		}
	}

	bool MenuItem_NewScene::OnKeyPressedImpl(KeyPressedEvent& e)
	{
		if (m_ShortcutName != "CTRL+N" || !GetOwningDockspace()->IsDockspaceFocused()) return false;

		bool bIsCtrlPressed = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		switch (e.GetKeyCode())
		{
			case Key::N:
			{
				if (bIsCtrlPressed)
				{
					m_OwningEditor->CreateNewScene();
				}
				break;
			}
		}

		return true;
	}

	void MenuItem_NewScene::OnMenuItemActivated()
	{
		m_OwningEditor->CreateNewScene();
	}

	bool MenuItem_OpenScene::OnKeyPressedImpl(KeyPressedEvent& e)
	{
		if (m_ShortcutName != "CTRL+O" || !GetOwningDockspace()->IsDockspaceFocused()) return false;

		bool bIsCtrlPressed = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		switch (e.GetKeyCode())
		{
			case Key::O:
			{
				if (bIsCtrlPressed)
				{
					m_OwningEditor->OpenScene();
				}
				break;
			}
		}

		return true;
	}

	void MenuItem_OpenScene::OnMenuItemActivated()
	{
		m_OwningEditor->OpenScene();
	}

	bool MenuItem_SaveScene::OnKeyPressedImpl(KeyPressedEvent& e)
	{
		if (m_ShortcutName != "CTRL+S" || !GetOwningDockspace()->IsDockspaceFocused()) return false;

		bool bIsCtrlPressed = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		switch (e.GetKeyCode())
		{
			case Key::S:
			{
				if (bIsCtrlPressed)
				{
					m_OwningEditor->SaveScene();
				}
				break;
			}
		}

		return true;
	}

	void MenuItem_SaveScene::OnMenuItemActivated()
	{
		m_OwningEditor->SaveScene();
	}

	bool MenuItem_SaveSceneAs::OnKeyPressedImpl(KeyPressedEvent& e)
	{
		if (m_ShortcutName != "CTRL+ALT+S" || !GetOwningDockspace()->IsDockspaceFocused()) return false;

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
						m_OwningEditor->SaveSceneAs();
					}
				}
				break;
			}
		}

		return true;
	}

	void MenuItem_SaveSceneAs::OnMenuItemActivated()
	{
		m_OwningEditor->SaveSceneAs();
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
		EditorManager::Get().RebuildLayoutForAllEditors();
	}

	void MenuItem_Snapshot::OnMenuItemActivated()
	{
		const std::string& filePath = m_OwningEditor->GetScene()->GetPath();
		// This may be null e.g. default particle system
		if (filePath.empty()) return;

		SceneViewportPanel* viewportPanel = GetOwningDockspace()->GetPanel<SceneViewportPanel>(GetOwningDockspace()->GetViewportPanelType());
		std::string snapshotName = filePath + ".png";
		viewportPanel->Snapshot(snapshotName, 256);
	}

}

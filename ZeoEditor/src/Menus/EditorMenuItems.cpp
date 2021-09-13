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
		: MenuItemBase(owningEditor, EditorUtils::GetEditorName(editorType), shortcutName)
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
		: MenuItemBase(owningEditor, EditorUtils::GetPanelName(panelType), shortcutName)
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

	bool MenuItem_NewAsset::OnKeyPressedImpl(KeyPressedEvent& e)
	{
		if (m_ShortcutName != "CTRL+N" || !GetOwningDockspace()->IsDockspaceFocused()) return false;

		bool bIsCtrlPressed = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		if (e.GetKeyCode() == Key::N && bIsCtrlPressed)
		{
			m_OwningEditor->NewAsset();
			return true;
		}

		return false;
	}

	void MenuItem_NewAsset::OnMenuItemActivated()
	{
		m_OwningEditor->NewAsset();
	}

	bool MenuItem_LoadAsset::OnKeyPressedImpl(KeyPressedEvent& e)
	{
		if (m_ShortcutName != "CTRL+O" || !GetOwningDockspace()->IsDockspaceFocused()) return false;

		bool bIsCtrlPressed = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		if (e.GetKeyCode() == Key::O && bIsCtrlPressed)
		{
			m_OwningEditor->LoadAsset();
			return true;
		}

		return false;
	}

	void MenuItem_LoadAsset::OnMenuItemActivated()
	{
		m_OwningEditor->LoadAsset();
	}

	bool MenuItem_SaveAsset::OnKeyPressedImpl(KeyPressedEvent& e)
	{
		if (m_ShortcutName != "CTRL+S" || !GetOwningDockspace()->IsDockspaceFocused()) return false;

		bool bIsCtrlPressed = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		if (e.GetKeyCode() == Key::S && bIsCtrlPressed)
		{
			m_OwningEditor->SaveAsset();
			return true;
		}

		return false;
	}

	void MenuItem_SaveAsset::OnMenuItemActivated()
	{
		m_OwningEditor->SaveAsset();
	}

	bool MenuItem_SaveAssetAs::OnKeyPressedImpl(KeyPressedEvent& e)
	{
		if (m_ShortcutName != "CTRL+ALT+S" || !GetOwningDockspace()->IsDockspaceFocused()) return false;

		bool bIsCtrlPressed = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		bool bIsAltPressed = Input::IsKeyPressed(Key::LeftAlt) || Input::IsKeyPressed(Key::RightAlt);
		if (e.GetKeyCode() == Key::S && bIsCtrlPressed && bIsAltPressed)
		{
			m_OwningEditor->SaveAssetAs();
			return true;
		}

		return false;
	}

	void MenuItem_SaveAssetAs::OnMenuItemActivated()
	{
		m_OwningEditor->SaveAssetAs();
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
		const std::string assetPath = m_OwningEditor->GetAssetPath();
		// This may be null e.g. default particle system
		if (assetPath.empty()) return;

		SceneViewportPanel* viewportPanel = GetOwningDockspace()->GetPanel<SceneViewportPanel>(GetOwningDockspace()->GetViewportPanelType());
		viewportPanel->Snapshot(assetPath, 256);
	}

}

#include "Menus/EditorMenuItems.h"

#include <imgui.h>

#include "Editors/EditorBase.h"
#include "Engine/Core/Input.h"
#include "Panels/EditorViewPanelBase.h"
#include "EditorUIRenderers/EditorUIRendererBase.h"

namespace ZeoEngine {

	MenuItemBase::MenuItemBase(const Ref<EditorBase>& contextEditor, const char* menuItemName, const char* shortcutName)
		: m_ContextEditor(contextEditor)
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

	Ref<EditorUIRendererBase> MenuItemBase::GetContextEditorUIRenderer() const
	{
		return m_ContextEditor->GetEditorUIRenderer();
	}

	bool MenuItemBase::OnKeyPressed(KeyPressedEvent& e)
	{
		if (m_ShortcutName.empty() || e.GetRepeatCount() > 0) return false;

		return OnKeyPressedImpl(e);
	}

	MenuItem_Seperator::MenuItem_Seperator(const Ref<EditorBase>& contextEditor, const char* menuItemName)
		: MenuItemBase(contextEditor, menuItemName)
	{
	}

	void MenuItem_Seperator::OnImGuiRender()
	{
		ImGui::Separator();
	}

	bool MenuItem_NewAsset::OnKeyPressedImpl(KeyPressedEvent& e)
	{
		if (m_ShortcutName != "CTRL+N" || !GetContextEditorUIRenderer()->IsEditorFocused()) return false;

		bool bIsCtrlPressed = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		if (e.GetKeyCode() == Key::N && bIsCtrlPressed)
		{
			m_ContextEditor->NewScene();
			return true;
		}

		return false;
	}

	void MenuItem_NewAsset::OnMenuItemActivated()
	{
		m_ContextEditor->NewScene();
	}

	bool MenuItem_LoadAsset::OnKeyPressedImpl(KeyPressedEvent& e)
	{
		if (m_ShortcutName != "CTRL+O" || !GetContextEditorUIRenderer()->IsEditorFocused()) return false;

		bool bIsCtrlPressed = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		if (e.GetKeyCode() == Key::O && bIsCtrlPressed)
		{
			m_ContextEditor->LoadScene();
			return true;
		}

		return false;
	}

	void MenuItem_LoadAsset::OnMenuItemActivated()
	{
		m_ContextEditor->LoadScene();
	}

	bool MenuItem_SaveAsset::OnKeyPressedImpl(KeyPressedEvent& e)
	{
		if (m_ShortcutName != "CTRL+S" || !GetContextEditorUIRenderer()->IsEditorFocused()) return false;

		bool bIsCtrlPressed = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		if (e.GetKeyCode() == Key::S && bIsCtrlPressed)
		{
			m_ContextEditor->SaveScene();
			return true;
		}

		return false;
	}

	void MenuItem_SaveAsset::OnMenuItemActivated()
	{
		m_ContextEditor->SaveScene();
	}

	bool MenuItem_SaveAssetAs::OnKeyPressedImpl(KeyPressedEvent& e)
	{
		if (m_ShortcutName != "CTRL+ALT+S" || !GetContextEditorUIRenderer()->IsEditorFocused()) return false;

		bool bIsCtrlPressed = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		bool bIsAltPressed = Input::IsKeyPressed(Key::LeftAlt) || Input::IsKeyPressed(Key::RightAlt);
		if (e.GetKeyCode() == Key::S && bIsCtrlPressed && bIsAltPressed)
		{
			m_ContextEditor->SaveSceneAs();
			return true;
		}

		return false;
	}

	void MenuItem_SaveAssetAs::OnMenuItemActivated()
	{
		m_ContextEditor->SaveSceneAs();
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
		const std::string assetPath = m_ContextEditor->GetAsset()->GetID();
		// This may be null e.g. default particle system
		if (assetPath.empty()) return;

		auto viewPanel = GetContextEditorUIRenderer()->GetViewPanel();
		viewPanel->Snapshot(assetPath, 256);
	}

}

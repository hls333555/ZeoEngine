#include "Menus/EditorMenuItems.h"

#include <imgui.h>

#include "Editors/EditorBase.h"
#include "Engine/Core/Input.h"
#include "Panels/EditorViewPanelBase.h"
#include "EditorUIRenderers/EditorUIRendererBase.h"
#include "Engine/Asset/AssetRegistry.h"

namespace ZeoEngine {

	MenuItemBase::MenuItemBase(const Weak<EditorBase>& contextEditor, std::string menuItemName, std::string shortcutName)
		: m_ContextEditor(contextEditor)
		, m_MenuItemName(std::move(menuItemName)), m_ShortcutName(std::move(shortcutName))
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

	EditorUIRendererBase& MenuItemBase::GetContextEditorUIRenderer() const
	{
		return *GetContextEditor()->GetEditorUIRenderer();
	}

	bool MenuItemBase::OnKeyPressed(KeyPressedEvent& e)
	{
		if (m_ShortcutName.empty() || e.GetRepeatCount() > 0) return false;

		return OnKeyPressedImpl(e);
	}

	MenuItem_Separator::MenuItem_Separator(const Weak<EditorBase>& contextEditor, std::string menuItemName)
		: MenuItemBase(contextEditor, std::move(menuItemName))
	{
	}

	void MenuItem_Separator::OnImGuiRender()
	{
		ImGui::Separator();
	}

	bool MenuItem_NewAsset::OnKeyPressedImpl(KeyPressedEvent& e)
	{
		if (m_ShortcutName != "CTRL+N" || !GetContextEditorUIRenderer().IsEditorFocused()) return false;

		const bool bIsCtrlPressed = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		if (e.GetKeyCode() == Key::N && bIsCtrlPressed)
		{
			GetContextEditor()->NewDefaultScene();
			return true;
		}

		return false;
	}

	void MenuItem_NewAsset::OnMenuItemActivated()
	{
		GetContextEditor()->NewDefaultScene();
	}

	bool MenuItem_LoadAsset::OnKeyPressedImpl(KeyPressedEvent& e)
	{
		if (m_ShortcutName != "CTRL+O" || !GetContextEditorUIRenderer().IsEditorFocused()) return false;

		const bool bIsCtrlPressed = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		if (e.GetKeyCode() == Key::O && bIsCtrlPressed)
		{
			GetContextEditor()->LoadScene();
			return true;
		}

		return false;
	}

	void MenuItem_LoadAsset::OnMenuItemActivated()
	{
		GetContextEditor()->LoadScene();
	}

	bool MenuItem_SaveAsset::OnKeyPressedImpl(KeyPressedEvent& e)
	{
		if (m_ShortcutName != "CTRL+S" || !GetContextEditorUIRenderer().IsEditorFocused()) return false;

		const bool bIsCtrlPressed = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		const bool bIsAltPressed = Input::IsKeyPressed(Key::LeftAlt) || Input::IsKeyPressed(Key::RightAlt);
		if (e.GetKeyCode() == Key::S && bIsCtrlPressed && !bIsAltPressed)
		{
			GetContextEditor()->SaveScene();
			return true;
		}

		return false;
	}

	void MenuItem_SaveAsset::OnMenuItemActivated()
	{
		GetContextEditor()->SaveScene();
	}

	bool MenuItem_SaveAssetAs::OnKeyPressedImpl(KeyPressedEvent& e)
	{
		if (m_ShortcutName != "CTRL+ALT+S" || !GetContextEditorUIRenderer().IsEditorFocused()) return false;

		const bool bIsCtrlPressed = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		const bool bIsAltPressed = Input::IsKeyPressed(Key::LeftAlt) || Input::IsKeyPressed(Key::RightAlt);
		if (e.GetKeyCode() == Key::S && bIsCtrlPressed && bIsAltPressed)
		{
			GetContextEditor()->SaveSceneAs();
			return true;
		}

		return false;
	}

	void MenuItem_SaveAssetAs::OnMenuItemActivated()
	{
		GetContextEditor()->SaveSceneAs();
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
		const auto metadata = AssetRegistry::Get().GetAssetMetadata(GetContextEditor()->GetAsset()->GetHandle());
		const auto viewPanel = GetContextEditorUIRenderer().GetViewPanel();
		viewPanel->Snapshot(metadata, 256);
	}

}

#include "Menus/EditorMenuItem.h"

#include <imgui.h>

#include "Menus/EditorMenu.h"
#include "Core/WindowManager.h"
#include "Engine/Core/Input.h"
#include "Panels/SceneViewportPanel.h"
#include "Dockspaces/EditorDockspace.h"

namespace ZeoEngine {

	EditorMenuItem::EditorMenuItem(EditorMenu* context, const std::string& menuItemName, const std::string& shortcutName)
		: m_Context(context)
		, m_MenuItemName(menuItemName), m_ShortcutName(shortcutName)
	{
	}

	void EditorMenuItem::OnImGuiRender()
	{
		if (ImGui::MenuItem(m_MenuItemName.c_str(), m_ShortcutName.c_str(), m_bSelected, m_bEnabled))
		{
			OnMenuItemActivated();
		}
	}

	void EditorMenuItem::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>(ZE_BIND_EVENT_FUNC(EditorMenuItem::OnKeyPressed));
	}

	bool EditorMenuItem::OnKeyPressed(KeyPressedEvent& e)
	{
		if (m_ShortcutName.empty() || e.GetRepeatCount() > 0) return false;

		return OnKeyPressedOverride(e);
	}

	EditorDockspace* EditorMenuItem::GetEditorContext() const
	{
		return m_Context->GetContext();
	}

	const Ref<Scene>& EditorMenuItem::GetScene() const
	{
		return GetEditorContext()->GetScene();
	}

	MenuItem_Seperator::MenuItem_Seperator(EditorMenu* context, const std::string& menuItemName)
		: EditorMenuItem(context, menuItemName)
	{
	}

	void MenuItem_Seperator::OnImGuiRender()
	{
		ImGui::Separator();
	}

	MenuItem_ToggleEditor::MenuItem_ToggleEditor(EditorMenu* context, EditorDockspaceType dockspaceType, const std::string& shortcutName)
		: EditorMenuItem(context, GetDockspaceName(dockspaceType), shortcutName)
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

		EditorMenuItem::OnImGuiRender();
	}

	void MenuItem_ToggleEditor::OnMenuItemActivated()
	{
		if (!m_bSelected)
		{
			DockspaceManager::Get().ToggleDockspace(m_DockspaceType, true);
		}
	}

	MenuItem_TogglePanel::MenuItem_TogglePanel(EditorMenu* context, EditorPanelType panelType, const std::string& shortcutName)
		: EditorMenuItem(context, GetPanelName(panelType), shortcutName)
		, m_PanelType(panelType)
	{
	}

	void MenuItem_TogglePanel::OnImGuiRender()
	{
		if (!m_bSelected)
		{
			auto* panel = GetEditorContext()->GetPanel(m_PanelType);
			if (panel)
			{
				m_bSelected = panel->GetShowPtr();
			}
		}

		EditorMenuItem::OnImGuiRender();
	}

	void MenuItem_TogglePanel::OnMenuItemActivated()
	{
		if (!m_bSelected)
		{
			GetEditorContext()->TogglePanel(m_PanelType, true);
		}
	}

	bool MenuItem_NewScene::OnKeyPressedOverride(KeyPressedEvent& e)
	{
		if (m_ShortcutName != "CTRL+N" || !GetEditorContext()->IsDockspaceFocused()) return false;

		bool bIsCtrlPressed = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		switch (e.GetKeyCode())
		{
			case Key::N:
			{
				if (bIsCtrlPressed)
				{
					GetEditorContext()->CreateNewScene();
				}
				break;
			}
			default:
				break;
		}

		return true;
	}

	void MenuItem_NewScene::OnMenuItemActivated()
	{
		GetEditorContext()->CreateNewScene();
	}

	bool MenuItem_OpenScene::OnKeyPressedOverride(KeyPressedEvent& e)
	{
		if (m_ShortcutName != "CTRL+O" || !GetEditorContext()->IsDockspaceFocused()) return false;

		bool bIsCtrlPressed = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		switch (e.GetKeyCode())
		{
			case Key::O:
			{
				if (bIsCtrlPressed)
				{
					GetEditorContext()->OpenScene();
				}
				break;
			}
			default:
				break;
		}

		return true;
	}

	void MenuItem_OpenScene::OnMenuItemActivated()
	{
		GetEditorContext()->OpenScene();
	}

	bool MenuItem_SaveScene::OnKeyPressedOverride(KeyPressedEvent& e)
	{
		if (m_ShortcutName != "CTRL+S" || !GetEditorContext()->IsDockspaceFocused()) return false;

		bool bIsCtrlPressed = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		switch (e.GetKeyCode())
		{
			case Key::S:
			{
				if (bIsCtrlPressed)
				{
					GetEditorContext()->SaveScene();
				}
				break;
			}
			default:
				break;
		}

		return true;
	}

	void MenuItem_SaveScene::OnMenuItemActivated()
	{
		GetEditorContext()->SaveScene();
	}

	bool MenuItem_SaveSceneAs::OnKeyPressedOverride(KeyPressedEvent& e)
	{
		if (m_ShortcutName != "CTRL+ALT+S" || !GetEditorContext()->IsDockspaceFocused()) return false;

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
						GetEditorContext()->SaveSceneAs();
					}
				}
				break;
			}
			default:
				break;
		}

		return true;
	}

	void MenuItem_SaveSceneAs::OnMenuItemActivated()
	{
		GetEditorContext()->SaveSceneAs();
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
		const std::string filePath = GetEditorContext()->GetScene()->GetPath();
		// This may be null e.g. default particle system
		if (filePath == "") return;

		SceneViewportPanel* viewportPanel = GetEditorContext()->GetPanel<SceneViewportPanel>(GetEditorContext()->GetViewportPanelType());
		std::string snapshotName = filePath + ".png";
		viewportPanel->Snapshot(snapshotName, 256);
	}

}

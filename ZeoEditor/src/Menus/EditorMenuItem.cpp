#include "Menus/EditorMenuItem.h"

#include <imgui.h>

#include "Engine/Core/Application.h"
#include "EditorLayer.h"
#include "Dockspaces/MainDockspace.h"
#include "Engine/Core/Input.h"
#include "Panels/SceneViewportPanel.h"

namespace ZeoEngine {

	EditorMenuItem::EditorMenuItem(const std::string& menuItemName, const std::string& shortcutName)
		: m_MenuItemName(menuItemName), m_ShortcutName(shortcutName)
	{
	}

	void EditorMenuItem::OnImGuiRender()
	{
		if (ImGui::MenuItem(m_MenuItemName.c_str(), m_ShortcutName.c_str(), false, m_bEnabled))
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

		bool bIsCtrlPressed = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		bool bIsAltPressed = Input::IsKeyPressed(Key::LeftAlt) || Input::IsKeyPressed(Key::RightAlt);
		switch (e.GetKeyCode())
		{
			case Key::N:
			{
				if (bIsCtrlPressed && m_ShortcutName == "CTRL+N")
				{
					GetEditorContext()->CreateNewScene();
				}
				break;
			}
			case Key::O:
			{
				if (bIsCtrlPressed && m_ShortcutName == "CTRL+O")
				{
					GetEditorContext()->OpenScene();
				}
				break;
			}
			case Key::S:
			{
				if (bIsCtrlPressed)
				{
					if (bIsAltPressed && m_ShortcutName == "CTRL+ALT+S")
					{
						GetEditorContext()->SaveSceneAs();
					}
					else if (m_ShortcutName == "CTRL+S")
					{
						GetEditorContext()->SaveScene();
					}

				}
				break;
			}
			default:
				break;
		}

		return true;
	}

	EditorDockspace* EditorMenuItem::GetEditorContext() const
	{
		return m_Context->GetContext();
	}

	const Ref<Scene>& EditorMenuItem::GetScene() const
	{
		return GetEditorContext()->GetScene();
	}

	MenuItem_Seperator::MenuItem_Seperator(const std::string& menuItemName)
		: EditorMenuItem(menuItemName, std::string())
	{
	}

	void MenuItem_Seperator::OnImGuiRender()
	{
		ImGui::Separator();
	}

	MenuItem_ToggleWindow::MenuItem_ToggleWindow(const std::string windowName, const std::string& shortcutName, bool* bShowWindowPtr)
		: EditorMenuItem(windowName, shortcutName)
		, m_bShowWindowPtr(bShowWindowPtr)
	{
	}

	void MenuItem_ToggleWindow::OnImGuiRender()
	{
		ImGui::MenuItem(m_MenuItemName.c_str(), m_ShortcutName.c_str(), m_bShowWindowPtr, m_bEnabled);
	}

	void MenuItem_NewScene::OnMenuItemActivated()
	{
		GetEditorContext()->CreateNewScene();
	}

	void MenuItem_OpenScene::OnMenuItemActivated()
	{
		GetEditorContext()->OpenScene();
	}

	void MenuItem_SaveScene::OnMenuItemActivated()
	{
		GetEditorContext()->SaveScene();
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
		EditorLayer* editorLayer = Application::Get().FindLayer<EditorLayer>();
		editorLayer->RebuildDockLayout();
	}

	void MenuItem_Snapshot::OnMenuItemActivated()
	{
		const std::string filePath = GetEditorContext()->GetScene()->GetPath();
		// This may be null e.g. default particle system
		if (filePath == "") return;

		SceneViewportPanel* viewportPanel = GetEditorContext()->GetPanelByType<SceneViewportPanel>(GetEditorContext()->GetViewportPanelType());
		std::string snapshotName = filePath + ".png";
		viewportPanel->Snapshot(snapshotName, 256);
	}

}

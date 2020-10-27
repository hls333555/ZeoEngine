#include "Menus/EditorMenuItem.h"

#include <imgui.h>

#include "Engine/Core/Application.h"
#include "EditorLayer.h"
#include "Engine/Core/SceneSerializer.h"
#include "Dockspaces/MainDockspace.h"
#include "Engine/Utils/PlatformUtils.h"
#include "Engine/Core/Input.h"

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

	static void NewScene(EditorDockspace* editorContext)
	{
		editorContext->CreateNewScene();
		// Clear selected entity
		editorContext->SetContextEntity({});
	}

	static void OpenScene(EditorDockspace* editorContext)
	{
		auto filePath = FileDialogs::OpenFile("Zeo Scene (*.zscene)\0*.zscene\0");
		if (!filePath) return;

		NewScene(editorContext);
		// Save scene path
		editorContext->GetScene()->SetPath(*filePath);

		SceneSerializer serializer(editorContext->GetScene());
		serializer.Deserialize(*filePath);
	}

	static void SaveSceneAs(EditorDockspace* editorContext)
	{
		auto filePath = FileDialogs::SaveFile("Zeo Scene (*.zscene)\0*.zscene\0");
		if (!filePath) return;

		// Save scene path
		editorContext->GetScene()->SetPath(*filePath);

		SceneSerializer serializer(editorContext->GetScene());
		serializer.Serialize(*filePath);
	}

	static void SaveScene(EditorDockspace* editorContext)
	{
		const std::string scenePath = editorContext->GetScene()->GetPath();
		if (scenePath.empty())
		{
			SaveSceneAs(editorContext);
		}
		else
		{
			SceneSerializer serializer(editorContext->GetScene());
			serializer.Serialize(scenePath);
		}
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
					NewScene(GetEditorContext());
				}
				break;
			}
			case Key::O:
			{
				if (bIsCtrlPressed && m_ShortcutName == "CTRL+O")
				{
					OpenScene(GetEditorContext());
				}
				break;
			}
			case Key::S:
			{
				if (bIsCtrlPressed)
				{
					if (bIsAltPressed && m_ShortcutName == "CTRL+ALT+S")
					{
						SaveSceneAs(GetEditorContext());
					}
					else if (m_ShortcutName == "CTRL+S")
					{
						SaveScene(GetEditorContext());
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

	MenuItem_ToggleWindow::MenuItem_ToggleWindow(const std::string& menuItemName, const std::string& shortcutName, bool* bShowWindowPtr)
		: EditorMenuItem(menuItemName, shortcutName)
		, m_bShowWindowPtr(bShowWindowPtr)
	{
	}

	void MenuItem_ToggleWindow::OnImGuiRender()
	{
		ImGui::MenuItem(m_MenuItemName.c_str(), m_ShortcutName.c_str(), m_bShowWindowPtr, m_bEnabled);
	}

	void MenuItem_NewScene::OnMenuItemActivated()
	{
		NewScene(GetEditorContext());
	}

	void MenuItem_OpenScene::OnMenuItemActivated()
	{
		OpenScene(GetEditorContext());
	}

	void MenuItem_SaveScene::OnMenuItemActivated()
	{
		SaveScene(GetEditorContext());
	}

	void MenuItem_SaveSceneAs::OnMenuItemActivated()
	{
		SaveSceneAs(GetEditorContext());
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
		editorLayer->GetDockspaceManager().RebuildDockLayout();
	}

}

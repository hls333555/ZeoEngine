#include "Menus/EditorMenuItem.h"

#include <imgui.h>

#include "Engine/Core/Application.h"
#include "EditorLayer.h"
#include "Engine/Core/SceneSerializer.h"
#include "Dockspaces/MainDockspace.h"

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

	MainDockspace* EditorMenuItem::GetEditorContext() const
	{
		return m_Context->GetContext<MainDockspace>();
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
		
	}

	void MenuItem_OpenScene::OnMenuItemActivated()
	{
		// Reset scene
		GetScene()->ResetScene();
		// Clear selected entity
		GetEditorContext()->GetSeletedEntity() = {};
		SceneSerializer serializer(GetScene());
		serializer.Deserialize("assets/scenes/Untitled.zscene");
	}

	void MenuItem_SaveScene::OnMenuItemActivated()
	{
		SceneSerializer serializer(GetScene());
		serializer.Serialize("assets/scenes/Untitled.zscene");
	}

	void MenuItem_SaveSceneAs::OnMenuItemActivated()
	{
		
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

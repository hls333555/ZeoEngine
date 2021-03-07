#pragma once

#include <string>

#include "Engine/GameFramework/Scene.h"
#include "Engine/Events/KeyEvent.h"
#include "Utils/EditorUtils.h"

namespace ZeoEngine {

	class EditorMenu;
	class EditorDockspace;

	class EditorMenuItem
	{
	public:
		EditorMenuItem() = delete;
		EditorMenuItem(EditorMenu* context, const std::string& menuItemName, const std::string& shortcutName = "");
		virtual ~EditorMenuItem() = default;

		virtual void OnImGuiRender();
		void OnEvent(Event& e);

		void SetEnabled(bool bEnabled) { m_bEnabled = bEnabled; }

	protected:
		EditorDockspace* GetEditorContext() const;
		const Ref<Scene>& GetScene() const;

	private:
		/** Handles key shortcuts. */
		bool OnKeyPressed(KeyPressedEvent& e);
		virtual bool OnKeyPressedOverride(KeyPressedEvent& e) { return false; };
		virtual void OnMenuItemActivated() = 0;

	protected:
		std::string m_MenuItemName;
		std::string m_ShortcutName;
		EditorMenu* m_Context;
		bool m_bEnabled = true;
	};

	class MenuItem_Seperator : public EditorMenuItem
	{
	public:
		explicit MenuItem_Seperator(EditorMenu* context, const std::string& menuItemName = "Seperator");

		virtual void OnImGuiRender() override;

	private:
		virtual void OnMenuItemActivated() override {}
	};

	class MenuItem_ToggleEditor : public EditorMenuItem
	{
	public:
		MenuItem_ToggleEditor(EditorMenu* context, EditorDockspaceType dockspaceType, const std::string& shortcutName = "");

		virtual void OnImGuiRender() override;

	private:
		virtual void OnMenuItemActivated() override {}

	private:
		EditorDockspaceType m_DockspaceType;
		bool* m_bShowPtr = nullptr;
	};

	class MenuItem_TogglePanel : public EditorMenuItem
	{
	public:
		MenuItem_TogglePanel(EditorMenu* context, EditorPanelType panelType, const std::string& shortcutName = "");

		virtual void OnImGuiRender() override;

	private:
		virtual void OnMenuItemActivated() override {}

	private:
		EditorPanelType m_PanelType;
		bool* m_bShowPtr = nullptr;
	};

	class MenuItem_NewScene : public EditorMenuItem
	{
	public:
		// Inherit parent's constructors
		using EditorMenuItem::EditorMenuItem;

	private:
		virtual bool OnKeyPressedOverride(KeyPressedEvent& e) override;
		virtual void OnMenuItemActivated() override;
	};

	class MenuItem_OpenScene : public EditorMenuItem
	{
	public:
		using EditorMenuItem::EditorMenuItem;

	private:
		virtual bool OnKeyPressedOverride(KeyPressedEvent& e) override;
		virtual void OnMenuItemActivated() override;
	};

	class MenuItem_SaveScene : public EditorMenuItem
	{
	public:
		using EditorMenuItem::EditorMenuItem;

	private:
		virtual bool OnKeyPressedOverride(KeyPressedEvent& e) override;
		virtual void OnMenuItemActivated() override;
	};

	class MenuItem_SaveSceneAs : public EditorMenuItem
	{
	public:
		using EditorMenuItem::EditorMenuItem;

	private:
		virtual bool OnKeyPressedOverride(KeyPressedEvent& e) override;
		virtual void OnMenuItemActivated() override;
	};

	class MenuItem_Undo : public EditorMenuItem
	{
	public:
		using EditorMenuItem::EditorMenuItem;

	private:
		virtual void OnMenuItemActivated() override;
	};

	class MenuItem_Redo : public EditorMenuItem
	{
	public:
		using EditorMenuItem::EditorMenuItem;

	private:
		virtual void OnMenuItemActivated() override;
	};

	class MenuItem_Copy : public EditorMenuItem
	{
	public:
		using EditorMenuItem::EditorMenuItem;

	private:
		virtual void OnMenuItemActivated() override;
	};

	class MenuItem_Paste : public EditorMenuItem
	{
	public:
		using EditorMenuItem::EditorMenuItem;

	private:
		virtual void OnMenuItemActivated() override;
	};

	class MenuItem_Cut : public EditorMenuItem
	{
	public:
		using EditorMenuItem::EditorMenuItem;

	private:
		virtual void OnMenuItemActivated() override;
	};
	
	class MenuItem_ResetLayout : public EditorMenuItem
	{
	public:
		using EditorMenuItem::EditorMenuItem;

	private:
		virtual void OnMenuItemActivated() override;
	};

	class MenuItem_Snapshot : public EditorMenuItem
	{
	public:
		using EditorMenuItem::EditorMenuItem;

	private:
		virtual void OnMenuItemActivated() override;
	};

}

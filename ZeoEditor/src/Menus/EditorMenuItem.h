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
		EditorMenuItem(const std::string& menuItemName, const std::string& shortcutName);
		virtual ~EditorMenuItem() = default;

		virtual void OnImGuiRender();
		void OnEvent(Event& e);

		void SetContext(EditorMenu* contextMenu) { m_Context = contextMenu; }
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
		bool m_bEnabled{ true };
	};

	class MenuItem_Seperator : public EditorMenuItem
	{
	public:
		explicit MenuItem_Seperator(const std::string& menuItemName = "Seperator");

		virtual void OnImGuiRender() override;

	private:
		virtual void OnMenuItemActivated() override {}
	};

	class MenuItem_ToggleWindow : public EditorMenuItem
	{
	public:
		MenuItem_ToggleWindow(const std::string windowName, const std::string& shortcutName, bool* bShowWindowPtr);

		virtual void OnImGuiRender() override;

	private:
		virtual void OnMenuItemActivated() override {}

	private:
		bool* m_bShowWindowPtr;
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

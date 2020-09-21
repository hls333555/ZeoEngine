#pragma once

#include <string>

namespace ZeoEngine {

	class EditorMenuItem
	{
	public:
		EditorMenuItem() = delete;
		EditorMenuItem(const std::string& menuItemName, const std::string& shortcutName);
		virtual ~EditorMenuItem() = default;

		virtual void OnImGuiRender();

		void SetEnabled(bool bEnabled) { m_bEnabled = bEnabled; }

	private:
		virtual void OnMenuItemActivated() = 0;

	protected:
		std::string m_MenuItemName;
		std::string m_ShortcutName; // TODO: Shortcut support
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
		MenuItem_ToggleWindow(const std::string& menuItemName, const std::string& shortcutName, bool* bShowWindowPtr);

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
		virtual void OnMenuItemActivated() override;
	};

	class MenuItem_OpenScene : public EditorMenuItem
	{
	public:
		using EditorMenuItem::EditorMenuItem;

	private:
		virtual void OnMenuItemActivated() override;
	};

	class MenuItem_SaveScene : public EditorMenuItem
	{
	public:
		using EditorMenuItem::EditorMenuItem;

	private:
		virtual void OnMenuItemActivated() override;
	};

	class MenuItem_SaveSceneAs : public EditorMenuItem
	{
	public:
		using EditorMenuItem::EditorMenuItem;

	private:
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

}

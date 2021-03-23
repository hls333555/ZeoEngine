#pragma once

#include <string>

#include "Engine/Events/KeyEvent.h"
#include "Core/EditorTypes.h"

namespace ZeoEngine {

	class DockspaceBase;

	class MenuItemBase
	{
		friend class EditorMenu;

	protected:
		MenuItemBase() = delete;
		MenuItemBase(DockspaceBase* context, const std::string& menuItemName, const std::string& shortcutName = "");
		virtual ~MenuItemBase() = default;
	
	public:
		virtual void OnImGuiRender();
		void OnEvent(Event& e);

		void SetEnabled(bool bEnabled) { m_bEnabled = bEnabled; }

	private:
		virtual void OnMenuItemActivated() {}

		/** Handles key shortcuts. */
		bool OnKeyPressed(KeyPressedEvent& e);
		virtual bool OnKeyPressedImpl(KeyPressedEvent& e) { return false; }

	protected:
		DockspaceBase* m_Context = nullptr;
		std::string m_MenuItemName;
		std::string m_ShortcutName;
		bool* m_bSelected = nullptr;
		bool m_bEnabled = true;
	};

	class MenuItem_Seperator : public MenuItemBase
	{
		friend class EditorMenu;

	private:
		explicit MenuItem_Seperator(DockspaceBase* context, const std::string& menuItemName = "Seperator");

	public:
		virtual void OnImGuiRender() override;
	};

	class MenuItem_ToggleEditor : public MenuItemBase
	{
		friend class EditorMenu;

	private:
		MenuItem_ToggleEditor(DockspaceBase* context, DockspaceType dockspaceType, const std::string& shortcutName = "");

	public:
		virtual void OnImGuiRender() override;

	private:
		virtual void OnMenuItemActivated() override;

	private:
		DockspaceType m_DockspaceType;
	};

	class MenuItem_TogglePanel : public MenuItemBase
	{
		friend class EditorMenu;

	private:
		MenuItem_TogglePanel(DockspaceBase* context, PanelType panelType, const std::string& shortcutName = "");

	public:
		virtual void OnImGuiRender() override;

	private:
		virtual void OnMenuItemActivated() override;

	private:
		PanelType m_PanelType;
	};

	class MenuItem_NewScene : public MenuItemBase
	{
	public:
		using MenuItemBase::MenuItemBase;

	private:
		virtual bool OnKeyPressedImpl(KeyPressedEvent& e) override;
		virtual void OnMenuItemActivated() override;
	};

	class MenuItem_OpenScene : public MenuItemBase
	{
	public:
		using MenuItemBase::MenuItemBase;

	private:
		virtual bool OnKeyPressedImpl(KeyPressedEvent& e) override;
		virtual void OnMenuItemActivated() override;
	};

	class MenuItem_SaveScene : public MenuItemBase
	{
	public:
		using MenuItemBase::MenuItemBase;

	private:
		virtual bool OnKeyPressedImpl(KeyPressedEvent& e) override;
		virtual void OnMenuItemActivated() override;
	};

	class MenuItem_SaveSceneAs : public MenuItemBase
	{
	public:
		using MenuItemBase::MenuItemBase;

	private:
		virtual bool OnKeyPressedImpl(KeyPressedEvent& e) override;
		virtual void OnMenuItemActivated() override;
	};

	class MenuItem_Undo : public MenuItemBase
	{
	public:
		using MenuItemBase::MenuItemBase;

	private:
		virtual void OnMenuItemActivated() override;
	};

	class MenuItem_Redo : public MenuItemBase
	{
	public:
		using MenuItemBase::MenuItemBase;

	private:
		virtual void OnMenuItemActivated() override;
	};

	class MenuItem_Copy : public MenuItemBase
	{
	public:
		using MenuItemBase::MenuItemBase;

	private:
		virtual void OnMenuItemActivated() override;
	};

	class MenuItem_Paste : public MenuItemBase
	{
	public:
		using MenuItemBase::MenuItemBase;

	private:
		virtual void OnMenuItemActivated() override;
	};

	class MenuItem_Cut : public MenuItemBase
	{
	public:
		using MenuItemBase::MenuItemBase;

	private:
		virtual void OnMenuItemActivated() override;
	};
	
	class MenuItem_ResetLayout : public MenuItemBase
	{
	public:
		using MenuItemBase::MenuItemBase;

	private:
		virtual void OnMenuItemActivated() override;
	};

	class MenuItem_Snapshot : public MenuItemBase
	{
	public:
		using MenuItemBase::MenuItemBase;

	private:
		virtual void OnMenuItemActivated() override;
	};

}

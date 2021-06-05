#pragma once

#include <string>

#include "Engine/Core/Core.h"
#include "Engine/Events/KeyEvent.h"
#include "Core/EditorTypes.h"

namespace ZeoEngine {

	class EditorBase;
	class DockspaceBase;

	class MenuItemBase
	{
		friend class EditorMenu;

	protected:
		MenuItemBase() = delete;
		MenuItemBase(EditorBase* owningEditor, const std::string& menuItemName, const std::string& shortcutName = "");
		virtual ~MenuItemBase() = default;
	
	public:
		virtual void OnImGuiRender();
		void OnEvent(Event& e);

		void SetEnabled(bool bEnabled) { m_bEnabled = bEnabled; }

	protected:
		DockspaceBase* GetOwningDockspace() const;

	private:
		virtual void OnMenuItemActivated() {}

		/** Handles key shortcuts. */
		bool OnKeyPressed(KeyPressedEvent& e);
		virtual bool OnKeyPressedImpl(KeyPressedEvent& e) { return false; }

	protected:
		EditorBase* m_OwningEditor = nullptr;
		std::string m_MenuItemName;
		std::string m_ShortcutName;
		bool* m_bSelected = nullptr;
		bool m_bEnabled = true;
	};

	class MenuItem_Seperator : public MenuItemBase
	{
		friend class EditorMenu;

	private:
		explicit MenuItem_Seperator(EditorBase* context, const std::string& menuItemName = "Seperator");

	public:
		virtual void OnImGuiRender() override;
	};

	class MenuItem_ToggleEditor : public MenuItemBase
	{
		friend class EditorMenu;

	private:
		MenuItem_ToggleEditor(EditorBase* owningEditor, EditorType editorType, const std::string& shortcutName = "");

	public:
		virtual void OnImGuiRender() override;

	private:
		virtual void OnMenuItemActivated() override;

	private:
		EditorType m_EditorType;
	};

	class MenuItem_TogglePanel : public MenuItemBase
	{
		friend class EditorMenu;

	private:
		MenuItem_TogglePanel(EditorBase* owningEditor, PanelType panelType, const std::string& shortcutName = "");

	public:
		virtual void OnImGuiRender() override;

	private:
		virtual void OnMenuItemActivated() override;

	private:
		PanelType m_PanelType;
	};

	class MenuItem_NewAsset : public MenuItemBase
	{
	public:
		using MenuItemBase::MenuItemBase;

	private:
		virtual bool OnKeyPressedImpl(KeyPressedEvent& e) override;
		virtual void OnMenuItemActivated() override;
	};

	class MenuItem_LoadAsset : public MenuItemBase
	{
	public:
		using MenuItemBase::MenuItemBase;

	private:
		virtual bool OnKeyPressedImpl(KeyPressedEvent& e) override;
		virtual void OnMenuItemActivated() override;
	};

	class MenuItem_SaveAsset : public MenuItemBase
	{
	public:
		using MenuItemBase::MenuItemBase;

	private:
		virtual bool OnKeyPressedImpl(KeyPressedEvent& e) override;
		virtual void OnMenuItemActivated() override;
	};

	class MenuItem_SaveAssetAs : public MenuItemBase
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

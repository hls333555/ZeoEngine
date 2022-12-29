#pragma once

#include <string>

#include "Engine/Events/KeyEvent.h"
#include "Core/Editor.h"

namespace ZeoEngine {

	class MenuItemBase
	{
	public:
		MenuItemBase(std::string menuItemName, std::string shortcutName = "");
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
		std::string m_MenuItemName;
		std::string m_ShortcutName;
		bool* m_bSelected = nullptr;
		bool m_bEnabled = true;
	};

	class MenuItem_Separator : public MenuItemBase
	{
	public:
		explicit MenuItem_Separator(std::string menuItemName = "Separator");

		virtual void OnImGuiRender() override;
	};

	class MenuItem_TogglePanelBase : public MenuItemBase
	{
	public:
		MenuItem_TogglePanelBase(std::string panelName, std::string shortcutName = "")
			: MenuItemBase(std::move(panelName), std::move(shortcutName))
		{
		}

		virtual void OnImGuiRender() override;
	};

	template<typename PanelClass>
	class MenuItem_TogglePanel : public MenuItem_TogglePanelBase
	{
	public:
		using MenuItem_TogglePanelBase::MenuItem_TogglePanelBase;

	private:
		virtual void OnMenuItemActivated() override
		{
			if (!m_bSelected)
			{
				// You can pass different panel constructor parameters by specializing this template class
				g_Editor->OpenPanel<PanelClass>(m_MenuItemName);
			}
		}
	};

	class MenuItem_NewLevel : public MenuItemBase
	{
	public:
		using MenuItemBase::MenuItemBase;

	private:
		virtual bool OnKeyPressedImpl(KeyPressedEvent& e) override;
		virtual void OnMenuItemActivated() override;
	};

	class MenuItem_LoadLevel : public MenuItemBase
	{
	public:
		using MenuItemBase::MenuItemBase;

	private:
		virtual bool OnKeyPressedImpl(KeyPressedEvent& e) override;
		virtual void OnMenuItemActivated() override;
	};

	class MenuItem_SaveLevel : public MenuItemBase
	{
	public:
		using MenuItemBase::MenuItemBase;

	private:
		virtual bool OnKeyPressedImpl(KeyPressedEvent& e) override;
		virtual void OnMenuItemActivated() override;
	};

	class MenuItem_SaveLevelAs : public MenuItemBase
	{
	public:
		using MenuItemBase::MenuItemBase;

	private:
		virtual bool OnKeyPressedImpl(KeyPressedEvent& e) override;
		virtual void OnMenuItemActivated() override;
	};

	class MenuItem_OpenProject : public MenuItemBase
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

}

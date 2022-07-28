#pragma once

#include <string>

#include "Engine/Core/Core.h"
#include "Engine/Events/KeyEvent.h"
#include "Core/EditorManager.h"

namespace ZeoEngine {

	class EditorBase;
	class EditorUIRendererBase;

	class MenuItemBase
	{
	public:
		MenuItemBase() = delete;
		MenuItemBase(const Weak<EditorBase>& contextEditor, const char* menuItemName, const char* shortcutName = "");
	protected:
		virtual ~MenuItemBase() = default;
	
	public:
		virtual void OnImGuiRender();
		void OnEvent(Event& e);

		void SetEnabled(bool bEnabled) { m_bEnabled = bEnabled; }

	protected:
		Ref<EditorBase> GetContextEditor() const { return m_ContextEditor.lock(); }
		EditorUIRendererBase& GetContextEditorUIRenderer() const;

	private:
		virtual void OnMenuItemActivated() {}

		/** Handles key shortcuts. */
		bool OnKeyPressed(KeyPressedEvent& e);
		virtual bool OnKeyPressedImpl(KeyPressedEvent& e) { return false; }

	protected:
		Weak<EditorBase> m_ContextEditor;
		std::string m_MenuItemName;
		std::string m_ShortcutName;
		bool* m_bSelected = nullptr;
		bool m_bEnabled = true;
	};

	class MenuItem_Seperator : public MenuItemBase
	{
	public:
		explicit MenuItem_Seperator(const Weak<EditorBase>& contextEditor, const char* menuItemName = "Seperator");

		virtual void OnImGuiRender() override;
	};

	template<typename EditorClass>
	class MenuItem_ToggleEditor : public MenuItemBase
	{
	public:
		MenuItem_ToggleEditor(const Weak<EditorBase>& contextEditor, const char* editorName, const char* shortcutName = "")
			: MenuItemBase(contextEditor, editorName, shortcutName)
			, m_EditorName(editorName)
		{
		}

		virtual void OnImGuiRender() override
		{
			if (!m_bSelected)
			{
				if (auto editor = EditorManager::Get().GetEditor(m_EditorName))
				{
					m_bSelected = editor->GetShowPtr();
				}
			}

			MenuItemBase::OnImGuiRender();
		}

	private:
		virtual void OnMenuItemActivated() override
		{
			if (!m_bSelected)
			{
				EditorManager::Get().OpenEditor<EditorClass>(m_EditorName);
			}
		}

	private:
		std::string m_EditorName;
	};

	template<typename PanelClass>
	class MenuItem_TogglePanel : public MenuItemBase
	{
	public:
		MenuItem_TogglePanel(const Weak<EditorBase>& contextEditor, const char* panelName, const char* shortcutName = "")
			: MenuItemBase(contextEditor, panelName, shortcutName)
			, m_PanelName(panelName)
		{
		}

		virtual void OnImGuiRender() override
		{
			if (!m_bSelected)
			{
				if (auto panel = GetContextEditorUIRenderer().GetPanel(m_PanelName.c_str()))
				{
					m_bSelected = panel->GetShowPtr();
				}
			}

			MenuItemBase::OnImGuiRender();
		}

	private:
		virtual void OnMenuItemActivated() override
		{
			if (!m_bSelected)
			{
				GetContextEditorUIRenderer().OpenPanel<PanelClass>(m_PanelName.c_str());
			}
		}

	private:
		std::string m_PanelName;
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

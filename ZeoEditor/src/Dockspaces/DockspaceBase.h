#pragma once

#include "Core/MenuManager.h"
#include "Core/PanelManager.h"

namespace ZeoEngine {

	class EditorBase;
	class EditorMenu;
	class PanelBase;

	class DockspaceBase
	{
		friend class EditorBase;

	private:
		DockspaceBase() = delete;
		explicit DockspaceBase(EditorBase* owningEditor);
	protected:
		virtual ~DockspaceBase() = default;

	public:
		virtual void OnAttach() {}
		void OnUpdate(DeltaTime dt);
		void OnImGuiRender();
		void OnEvent(Event& e);

		bool IsDockspaceFocused() const { return m_bIsDockspaceFocused; }
		bool IsDockspaceHovered() const { return m_bIsDockspaceHovered; }

		virtual PanelType GetViewPanelType() const = 0;

	protected:
		EditorMenu& CreateMenu(const std::string& menuName);

		void CreatePanel(PanelType type);

	public:
		void OpenPanel(PanelType type);
		PanelBase* GetPanel(PanelType type);
		template<typename T>
		T* GetPanel(PanelType type)
		{
			return dynamic_cast<T*>(GetPanel(type));
		}

		void RebuildDockLayout() { m_bShouldRebuildDockLayout = true; }

	private:
		virtual void PreRenderDockspace();
		void RenderDockspace();
		virtual void BuildDockWindows(ImGuiID dockspaceID) = 0;

	protected:
		DockspaceSpec m_DockspaceSpec;

	private:
		EditorBase* m_OwningEditor = nullptr;

		bool m_bIsDockspaceFocused = false, m_bIsDockspaceHovered = false;
		bool m_bShouldRebuildDockLayout = false;

		MenuManager m_MenuManager;
		PanelManager m_PanelManager;

	};

}

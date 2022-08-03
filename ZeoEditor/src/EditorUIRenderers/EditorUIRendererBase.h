#pragma once

#include <imgui.h>

#include "Engine/Core/Core.h"
#include "Engine/Core/DeltaTime.h"
#include "Engine/Events/Event.h"
#include "Core/EditorTypes.h"

namespace ZeoEngine {

	class EditorBase;
	class EditorMenu;
	class PanelBase;
	class EditorViewPanelBase;

	class EditorUIRendererBase
	{
		friend EditorBase;

	public:
		EditorUIRendererBase() = delete;
		explicit EditorUIRendererBase(const Ref<EditorBase>& contextEditor);
		virtual ~EditorUIRendererBase() = default;

	public:
		virtual void OnAttach() {}
		void OnUpdate(DeltaTime dt);
		void OnImGuiRender();
		void OnEvent(Event& e);

		Ref<EditorBase> GetContextEditor() const { return m_ContextEditor.lock(); }
		template<typename T>
		Ref<T> GetContextEditor()
		{
			return std::dynamic_pointer_cast<T>(m_ContextEditor);
		}

		bool IsEditorFocused() const { return m_bIsEditorFocused; }
		bool IsEditorHovered() const { return m_bIsEditorHovered; }

	protected:
		EditorMenu& CreateMenu(std::string menuName);

		template<typename T, typename ... Args>
		Ref<T> CreatePanel(std::string panelName, Args&& ... args)
		{
			if (GetPanel(panelName))
			{
				ZE_CORE_ERROR("Failed to create {0}! Panel already exists!", panelName);
				return {};
			}

			Ref<T> panel = CreateRef<T>(panelName, m_ContextEditor, std::forward<Args>(args)...);
			m_Panels.emplace(std::move(panelName), panel);
			panel->OnAttach();
			panel->Open();
			return panel;
		}

	public:
		template<typename T>
		Ref<PanelBase> OpenPanel(std::string panelName)
		{
			Ref<PanelBase> panel = GetPanel(panelName);
			if (!panel)
			{
				panel = CreatePanel<T>(std::move(panelName));
			}

			panel->Open();
			return panel;
		}

		template<typename T = PanelBase>
		Ref<T> GetPanel(const std::string& panelName) const
		{
			const auto it = m_Panels.find(panelName);
			if (it == m_Panels.end()) return {};

			Ref<PanelBase> panel = it->second;
			if constexpr (std::is_same_v<T, PanelBase>)
			{
				return panel;
			}
			else
			{
				return std::dynamic_pointer_cast<T>(panel);
			}
		}

		Ref<EditorViewPanelBase> GetViewPanel() const;

		void RebuildDockLayout() { m_bShouldRebuildDockLayout = true; }

	private:
		void RenderDockspace();
		virtual void BuildDockWindows(ImGuiID dockspaceID) = 0;

		void RenderMainMenuBar();
		void RenderMenuBar();
		void RenderMenus();
		void OnEventMenus(Event& e);

		void UpdatePanels(DeltaTime dt);
		void RenderPanels();
		void OnEventPanels(Event& e);

	protected:
		DockspaceSpec m_DockspaceSpec;

	private:
		Weak<EditorBase> m_ContextEditor;

		bool m_bIsEditorFocused = false, m_bIsEditorHovered = false;
		bool m_bShouldRebuildDockLayout = false;

		std::vector<Ref<EditorMenu>> m_Menus;
		std::unordered_map<std::string, Ref<PanelBase>> m_Panels;

	};

}

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
	class ViewPanelBase;

	class EditorUIRendererBase
	{
	public:
		EditorUIRendererBase() = delete;
		explicit EditorUIRendererBase(const Ref<EditorBase>& contextEditor);
	protected:
		virtual ~EditorUIRendererBase() = default;

	public:
		virtual void OnAttach() {}
		void OnUpdate(DeltaTime dt);
		void OnImGuiRender();
		void OnEvent(Event& e);

		template<typename T = EditorBase>
		Ref<T> GetContextEditor()
		{
			if constexpr (std::is_same<T, EditorBase>::value)
			{
				return m_ContextEditor;
			}
			else
			{
				return std::dynamic_pointer_cast<T>(m_ContextEditor);
			}
		}

		bool IsEditorFocused() const { return m_bIsEditorFocused; }
		bool IsEditorHovered() const { return m_bIsEditorHovered; }

	protected:
		EditorMenu& CreateMenu(const char* menuName);

		template<typename T>
		Ref<T> CreatePanel(const char* panelName)
		{
			if (GetPanel(panelName))
			{
				ZE_CORE_ERROR("Failed to create {0}! Panel already exists!", panelName);
				return {};
			}

			Ref<T> panel = CreateRef<T>(panelName, m_ContextEditor);
			panel->OnAttach();
			m_Panels.emplace(panelName, panel);
			return panel;
		}

	public:
		template<typename T>
		Ref<PanelBase> OpenPanel(const char* panelName)
		{
			auto panel = GetPanel(panelName);
			if (!panel)
			{
				panel = CreatePanel<T>(panelName);
			}

			panel->Open();
			return panel;
		}

		template<typename T = PanelBase>
		Ref<T> GetPanel(const char* panelName)
		{
			auto it = m_Panels.find(panelName);
			if (it == m_Panels.end()) return {};

			Ref<PanelBase> panel = it->second;
			if constexpr (std::is_same<T, PanelBase>::value)
			{
				return panel;
			}
			else
			{
				return std::dynamic_pointer_cast<T>(panel);
			}
		}

		Ref<ViewPanelBase> GetViewPanel();

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
		Ref<EditorBase> m_ContextEditor;

		bool m_bIsEditorFocused = false, m_bIsEditorHovered = false;
		bool m_bShouldRebuildDockLayout = false;

		std::vector<Ref<EditorMenu>> m_Menus;
		std::unordered_map<std::string, Ref<PanelBase>> m_Panels;

	};

}

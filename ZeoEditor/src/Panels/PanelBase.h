#pragma once

#include "Core/EditorTypes.h"
#include "Engine/Core/Core.h"
#include "Engine/Core/DeltaTime.h"
#include "Engine/Events/Event.h"

namespace ZeoEngine {

	class PanelBase
	{
	public:
		PanelBase() = delete;
		explicit PanelBase(std::string panelName);
		virtual ~PanelBase() = default;

	public:
		virtual void OnAttach() {}
		virtual void OnDetach() {}
		void OnUpdate(DeltaTime dt);
		void OnImGuiRender();
		void OnEvent(Event& e);

		const std::string& GetPanelName() const { return m_PanelName; }

		void SetFlags(ImGuiWindowFlags flags) { m_PanelSpec.WindowFlags = flags; }
		void SetDisableClose(bool bDisable) { m_PanelSpec.bDisableClose = bDisable; }
		void SetPadding(const Vec2& padding) { m_PanelSpec.Padding = padding; }
		void SetInitialSize(const Vec2& size) { m_PanelSpec.InitialSize = size; }

		bool* GetShowPtr() { return &m_bShow; }
		bool IsPanelFocused() const { return m_bIsPanelFocused; }
		bool IsPanelHovered() const { return m_bIsPanelHovered; }

		void FocusPanel();

		void Toggle(bool bShow);


	private:
		virtual void ProcessUpdate(DeltaTime dt) {}
		virtual void ProcessDockspaceRender() {}
		virtual void ProcessRender() = 0;
		virtual void ProcessEvent(Event& e) {}

		virtual std::string GetPanelTitle() const { return m_PanelName; }

		virtual void OnPanelOpen() {}

	private:
		std::string m_PanelName;
		PanelSpec m_PanelSpec;

		bool m_bShow = true;
		bool m_bIsPanelFocused = false, m_bIsPanelHovered = false;
		bool m_bShouldFocusPanel = false;
	};

}

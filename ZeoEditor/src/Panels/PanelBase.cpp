#include "Panels/PanelBase.h"

#include "Engine/Core/Input.h"

namespace ZeoEngine {

	PanelBase::PanelBase(const char* panelName, const Ref<EditorBase>& contextEditor)
		: m_PanelName(panelName)
		, m_ContextEditor(contextEditor)
	{
	}

	void PanelBase::OnUpdate(DeltaTime dt)
	{
		if (!m_bShow) return;

		ProcessUpdate(dt);
	}

	void PanelBase::OnImGuiRender()
	{
		if (!m_bShow) return;

		ImGuiViewport* mainViewport = ImGui::GetMainViewport();
		ImVec2 centerPos{ mainViewport->Pos.x + mainViewport->Size.x / 2.0f, mainViewport->Pos.y + mainViewport->Size.y / 2.0f };
		ImGui::SetNextWindowPos(centerPos, ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
		ImGui::SetNextWindowSize(m_PanelSpec.InitialSize.Data, m_PanelSpec.InitialSize.Condition);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, m_PanelSpec.Padding);
		if (ImGui::Begin(GetPanelTitle().c_str(), &m_bShow, m_PanelSpec.WindowFlags))
		{
			m_bIsPanelFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows);
			m_bIsPanelHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);

			ProcessRender();
		}
		ImGui::PopStyleVar();

		// WORKAROUND: Click middle mouse button or right mouse button to focus panel
		if (m_bIsPanelHovered && (Input::IsMouseButtonPressed(Mouse::ButtonMiddle) || Input::IsMouseButtonPressed(Mouse::ButtonRight)))
		{
			FocusPanel();
		}

		if (m_bShouldFocusPanel)
		{
			ImGui::FocusWindow(ImGui::GetCurrentWindow());
			m_bShouldFocusPanel = false;
		}

		ImGui::End();
	}

	void PanelBase::OnEvent(Event& e)
	{
		if (!m_bShow) return;

		ProcessEvent(e);
	}

	void PanelBase::FocusPanel()
	{
		m_bShouldFocusPanel = true;
	}

	void PanelBase::Open()
	{
		m_bShow = true;
		OnPanelOpen();
	}

	void PanelBase::Close()
	{
		m_bShow = false;
	}

}

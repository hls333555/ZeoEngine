#include "Panels/PanelBase.h"

#include "Dockspaces/DockspaceBase.h"

namespace ZeoEngine {

	PanelBase::PanelBase(const PanelSpec& spec, DockspaceBase* context)
		: m_PanelSpec(spec)
		, m_Context(context)
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
		if (ImGui::Begin(GetPanelName(m_PanelSpec.Type), &m_bShow, m_PanelSpec.WindowFlags))
		{
			m_bIsPanelFocused = ImGui::IsWindowFocused();
			m_bIsPanelHovered = ImGui::IsWindowHovered();

			ProcessRender();
		}
		ImGui::PopStyleVar();

		ImGui::End();
	}

	void PanelBase::OnEvent(Event& e)
	{
		if (!m_bShow) return;

		ProcessEvent(e);
	}

}

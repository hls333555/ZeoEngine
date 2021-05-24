#include "Panels/PanelBase.h"

#include "Utils/EditorUtils.h"

namespace ZeoEngine {

	PanelBase::PanelBase(PanelType type, EditorBase* owningEditor)
		: m_PanelType(type)
		, m_OwningEditor(owningEditor)
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
		if (ImGui::Begin(EditorUtils::GetPanelName(m_PanelType), &m_bShow, m_PanelSpec.WindowFlags))
		{
			m_bIsPanelFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows);
			m_bIsPanelHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);

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

	void PanelBase::Open()
	{
		m_bShow = true;
	}

}

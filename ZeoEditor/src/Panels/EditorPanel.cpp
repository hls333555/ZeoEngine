#include "Panels/EditorPanel.h"

#include "Dockspaces/EditorDockspace.h"

namespace ZeoEngine {

	EditorPanel::EditorPanel(const EditorPanelSpec& spec, EditorDockspace* context)
		: m_PanelSpec(spec)
		, m_Context(context)
	{
	}

	void EditorPanel::OnImGuiRender()
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

			RenderPanel();
		}
		ImGui::PopStyleVar();

		ImGui::End();
	}

	const Ref<Scene>& EditorPanel::GetScene() const
	{
		return m_Context->GetScene();
	}

	const Ref<FrameBuffer>& EditorPanel::GetFrameBuffer() const
	{
		return m_Context->GetFrameBuffer();
	}

}

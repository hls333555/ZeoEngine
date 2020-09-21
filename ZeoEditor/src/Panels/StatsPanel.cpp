#include "Panels/StatsPanel.h"

#include <imgui/imgui.h>

#include "Engine/Renderer/Renderer2D.h"

namespace ZeoEngine {

	void StatsPanel::OnImGuiRender()
	{
		if (!m_bShow) return;

		EditorPanel::OnImGuiRender();

		if (ImGui::Begin(m_PanelName.c_str(), &m_bShow))
		{
			auto Stats = Renderer2D::GetStats();
			ImGui::Text("Draw Calls: %d", Stats.DrawCalls);
			ImGui::Text("Quads: %d", Stats.QuadCount);
			ImGui::Text("Vertices: %d", Stats.GetTotalVertexCount());
			ImGui::Text("Indices: %d", Stats.GetTotalIndexCount());
		}
		ImGui::End();
	}

}

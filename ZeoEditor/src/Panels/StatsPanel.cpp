#include "Panels/StatsPanel.h"

#include <imgui.h>

#include "Engine/Renderer/Renderer2D.h"

namespace ZeoEngine {

	void StatsPanel::ProcessRender()
	{
		auto& Stats = Renderer2D::GetStats();
		ImGui::Text("Draw Calls: %d", Stats.DrawCalls);
		ImGui::Text("Quads: %d", Stats.QuadCount);
		ImGui::Text("Vertices: %d", Stats.GetTotalVertexCount());
		ImGui::Text("Indices: %d", Stats.GetTotalIndexCount());

		std::string HoveredEntityName = Stats.HoveredEntity ? Stats.HoveredEntity.GetEntityName() : "NA";
		ImGui::Text("Hovered Entity: %s", HoveredEntityName.c_str());
	}

}

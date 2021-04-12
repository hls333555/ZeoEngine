#include "Panels/StatsPanel.h"

#include <imgui.h>

#include "Engine/Renderer/Renderer2D.h"

namespace ZeoEngine {

	void StatsPanel::ProcessRender()
	{
		auto& Stats = Renderer2D::GetStats();
		if (ImGui::TreeNodeEx("2D Renderer", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth))
		{
			ImGui::Text("Draw Calls: %d", Stats.DrawCalls);
			ImGui::Text("Quads: %d", Stats.QuadCount);
			ImGui::Text("Vertices: %d", Stats.GetTotalVertexCount());
			ImGui::Text("Indices: %d", Stats.GetTotalIndexCount());

			ImGui::TreePop();
		}

		if (ImGui::TreeNodeEx("Entity", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth))
		{
			std::string HoveredEntityName = Stats.HoveredEntity ? Stats.HoveredEntity.GetEntityName() : "NA";
			ImGui::Text("Hovered Entity: %s", HoveredEntityName.c_str());

			ImGui::TreePop();
		}
		
		if (ImGui::TreeNodeEx("Particle Template", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth))
		{
			for (const auto& [path, pTemplate] : ParticleLibrary::Get().GetAssetsMap())
			{
				std::ostringstream stringStream;
				stringStream << path << ": " << pTemplate->GetParticleSystemInstanceCount() << "reference(s)";
				std::string pTemplateInfo = stringStream.str();
				ImGui::Text(pTemplateInfo.c_str());
			}

			ImGui::TreePop();
		}
	}

}

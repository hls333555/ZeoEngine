#include "Panels/StatsPanel.h"

#include <imgui.h>

#include "Engine/Renderer/Renderer2D.h"

namespace ZeoEngine {

	void StatsPanel::OnAttach()
	{
		PanelBase::OnAttach();

		m_PanelSpec.WindowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking;
		m_PanelSpec.Padding = ImGui::GetStyle().WindowPadding;
		m_PanelSpec.InitialSize = { { 300.0f, 300.0f } };
	}

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
			std::string HoveredEntityName = Stats.HoveredEntity ? Stats.HoveredEntity.GetName() : "NA";
			ImGui::Text("Hovered Entity: %s", HoveredEntityName.c_str());

			ImGui::TreePop();
		}
		
		if (ImGui::TreeNodeEx("Particle Template", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth))
		{
			ParticleTemplateAssetLibrary::Get().ForEach([&](const entt::id_type id, const AssetHandle<ParticleTemplateAsset>& pTemplate)
			{
				std::ostringstream stringStream;
				stringStream << pTemplate->GetPath() << ": " << pTemplate->GetParticleSystemInstanceCount() << "reference(s)";
				std::string pTemplateInfo = stringStream.str();
				ImGui::Text(pTemplateInfo.c_str());
			});

			ImGui::TreePop();
		}
	}

}

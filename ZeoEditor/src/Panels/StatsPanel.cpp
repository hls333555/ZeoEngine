#include "Panels/StatsPanel.h"

#include <imgui.h>

#include "Engine/Core/Application.h"
#include "Engine/Renderer/Renderer2D.h"

namespace ZeoEngine {

	void StatsPanel::OnAttach()
	{
		PanelBase::OnAttach();

		SetFlags(ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize);
		SetPadding(ImGui::GetStyle().WindowPadding);
	}

	void StatsPanel::ProcessRender()
	{
		if (ImGui::TreeNodeEx("Performance", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth))
		{
			auto& profiler = Application::Get().GetPerformanceProfiler();
			const auto& perFrameData = profiler.GetPerFrameData();
			for (auto&& [name, time] : perFrameData)
			{
				ImGui::Text("%s: %.3fms\n", name, time);
			}
			profiler.Clear();

			ImGui::TreePop();
		}

		// TODO:
		auto& Stats = Renderer::GetStats();
		if (ImGui::TreeNodeEx("Renderer", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth))
		{
			ImGui::Text("Draw Calls: %d", Stats.DrawCalls);
			ImGui::Text("Quads: %d", Stats.QuadCount);
			ImGui::Text("Vertices: %d", Stats.GetTotalVertexCount());

			ImGui::TreePop();
		}

		if (ImGui::TreeNodeEx("Entity", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth))
		{
			const std::string hoveredEntityName = Stats.HoveredEntity ? Stats.HoveredEntity.GetName() : "NA";
			ImGui::Text("Hovered Entity: %s", hoveredEntityName.c_str());

			ImGui::TreePop();
		}

		if (ImGui::TreeNodeEx("ImGui", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth))
		{
			const auto* focusedWindow = ImGui::GetCurrentContext()->NavWindow;
			ImGui::Text("Focused Window: %s", focusedWindow ? focusedWindow->Name : "None");

			ImGui::TreePop();
		}

	}

}

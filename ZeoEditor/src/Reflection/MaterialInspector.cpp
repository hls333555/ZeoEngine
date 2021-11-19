#include "ZEpch.h"
#include "Reflection/MaterialInspector.h"

#include <imgui_internal.h>

#include "Engine/GameFramework/Components.h"
#include "Reflection/DataWidget.h"

namespace ZeoEngine {

	int32_t MaterialInspector::ProcessComponent()
	{
		int32_t result = ComponentInspector::ProcessComponent();

		ImGui::Separator();

		const auto& materialComp = GetOwnerEntity().GetComponent<MaterialPreviewComponent>();
		const auto& material = materialComp.Template->GetMaterial();
		// Sync table column separator with component inspector
		ImGui::PushOverrideID(GetTableID());
		if (ImGui::BeginTable("", 2, ImGuiTableFlags_Resizable))
		{
			for (const auto& uniform : material->GetDynamicUniforms())
			{
				ImGui::TableNextColumn();

				const char* name = uniform->Name.c_str();
				// Push name as id
				ImGui::PushID(name);
				{
					// Uniform name
					ImGui::TreeNodeEx(name, DefaultDataTreeNodeFlags);
					// Switch to the right column
					ImGui::TableNextColumn();
					// Align widget width to the right side
					ImGui::SetNextItemWidth(-1.0f);
					// Draw widget
					uniform->Draw();
				}
				ImGui::PopID();
			}

			ImGui::EndTable();
		}
		ImGui::PopID();
		
		return result;
	}

}

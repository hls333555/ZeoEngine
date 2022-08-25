#include "ZEpch.h"
#include "Reflection/MaterialInspector.h"

#include <imgui_internal.h>

#include "Engine/GameFramework/Components.h"
#include "Reflection/DataWidget.h"

namespace ZeoEngine {

	I32 MaterialInspector::ProcessComponent()
	{
		I32 result = ComponentInspector::ProcessComponent();

		ImGui::Separator();

		const auto& materialComp = GetOwnerEntity().GetComponent<MaterialPreviewComponent>();
		const auto& material = materialComp.MaterialAsset;
		const auto& dynamicData = material->GetDynamicData();
		const auto& dynamicDataCategoryLocations = material->GetDynamicDataCategoryLocations();
		const auto& dynamicBindableData = material->GetDynamicBindableData();

		// Draw resources
		{
			// Resource tree
			bool bIsTreeExpanded = ImGui::TreeNodeEx("Resource",
			   ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Selected | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_NoTreePushOnOpen/* Prevent indent of next row, which will affect column. */);
			if (bIsTreeExpanded)
			{
				// Sync table column separator with the component inspector
				ImGui::PushOverrideID(GetTableID());
				if (ImGui::BeginTable("", 2, ImGuiTableFlags_Resizable))
				{
					for (const auto& data : dynamicBindableData)
					{
						DrawUniformData(data);
					}

					ImGui::EndTable();
				}
				ImGui::PopID();
			}
		}

		// Draw uniform block datas
		{
			for (SizeT i = 0; i < dynamicDataCategoryLocations.size() - 1; ++i)
			{
				const auto location = dynamicDataCategoryLocations[i];
				// Uniform block tree
				bool bIsTreeExpanded = ImGui::TreeNodeEx(dynamicData[location]->Category.c_str(),
					ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Selected | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_NoTreePushOnOpen/* Prevent indent of next row, which will affect column. */);

				// Sync table column separator with the component inspector
				ImGui::PushOverrideID(GetTableID());
				if (ImGui::BeginTable("", 2, ImGuiTableFlags_Resizable))
				{
					const auto nextLocation = dynamicDataCategoryLocations[i + 1];
					for (SizeT j = location; j < nextLocation; ++j)
					{
						if (bIsTreeExpanded)
						{
							DrawUniformData(dynamicData[j]);
						}
					}

					ImGui::EndTable();
				}
				ImGui::PopID();
			}
		}

		// Certain macro value has been changed, reload dynamic data
		if (material->IsSnapshotDynamicDataAvailable())
		{
			material->ReloadShaderDataAndApplyDynamicData();
		}
		
		return result;
	}

	void MaterialInspector::DrawUniformData(const Ref<DynamicUniformDataBase>& data) const
	{
		ImGui::TableNextColumn();

		const char* name = data->Name.c_str();
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
			data->Draw();
		}
		ImGui::PopID();
	}

}

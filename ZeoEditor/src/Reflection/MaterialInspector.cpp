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
		const auto& shader = material->GetShader();
		const auto& uniforms = material->GetDynamicUniforms();
		const auto& bindableUniforms = material->GetDynamicBindableUniforms();
		const auto& uniformBlockDatas = shader->GetUniformBlockDatas();

		// Draw resources
		// Resources do not belong to uniform blocks so they do not need to draw tree header
		for (SizeT i = 0; i < shader->GetResourceCount(); ++i)
		{
			// Sync table column separator with the component inspector
			ImGui::PushOverrideID(GetTableID());
			if (ImGui::BeginTable("", 2, ImGuiTableFlags_Resizable))
			{
				DrawUniformData(bindableUniforms[i]);

				ImGui::EndTable();
			}
			ImGui::PopID();
		}

		// Draw uniform block datas
		for (const auto& [binding, uniformBlockData] : uniformBlockDatas)
		{
			// Uniform block tree
			bool bIsUniformTreeExpanded = ImGui::TreeNodeEx(uniformBlockData.Name.c_str(),
				ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Selected | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_NoTreePushOnOpen/* Prevent indent of next row, which will affect column. */);
			if (bIsUniformTreeExpanded)
			{
				// Sync table column separator with the component inspector
				ImGui::PushOverrideID(GetTableID());
				if (ImGui::BeginTable("", 2, ImGuiTableFlags_Resizable))
				{
					for (SizeT i = uniformBlockData.BeginIndex; i < uniformBlockData.EndIndex; ++i)
					{
						DrawUniformData(uniforms[i]);
					}

					ImGui::EndTable();
				}
				ImGui::PopID();
			}
		}
		
		return result;
	}

	void MaterialInspector::DrawUniformData(const Ref<DynamicUniformDataBase>& uniform) const
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

}

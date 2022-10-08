#include "ZEpch.h"
#include "Inspectors/MaterialInspector.h"

#include <imgui_internal.h>

#include "Engine/GameFramework/Components.h"

namespace ZeoEngine {

	void MaterialInspector::ProcessDraw(Entity entity)
	{
		ImGui::Separator();

		const auto& materialComp = entity.GetComponent<MaterialPreviewComponent>();
		const auto& material = materialComp.LoadedMaterial;
		const auto& dynamicFields = material->GetDynamicFields();
		const auto& dynamicFieldCategoryLocations = material->GetDynamicFieldCategoryLocations();
		const auto& dynamicBindableFields = material->GetDynamicBindableFields();

		const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Selected | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_NoTreePushOnOpen/* Prevent indent of next row, which will affect column. */;
		// Draw resources
		{
			// Resource tree
			bool bIsTreeExpanded = ImGui::TreeNodeEx("Resource", flags);
			if (bIsTreeExpanded)
			{
				// Sync table column separator with the component inspector
				ImGui::PushOverrideID(GetTableID());
				if (ImGui::BeginTable("", 2, ImGuiTableFlags_Resizable))
				{
					for (const auto& field : dynamicBindableFields)
					{
						DrawFieldWidget(field);
					}

					ImGui::EndTable();
				}
				ImGui::PopID();
			}
		}

		// Draw uniform block fields
		{
			for (SizeT i = 0; i < dynamicFieldCategoryLocations.size() - 1; ++i)
			{
				const auto location = dynamicFieldCategoryLocations[i];
				// Uniform block tree
				bool bIsTreeExpanded = ImGui::TreeNodeEx(dynamicFields[location]->Category.c_str(), flags);

				// Sync table column separator with the component inspector
				ImGui::PushOverrideID(GetTableID());
				if (ImGui::BeginTable("", 2, ImGuiTableFlags_Resizable))
				{
					const auto nextLocation = dynamicFieldCategoryLocations[i + 1];
					for (SizeT j = location; j < nextLocation; ++j)
					{
						if (bIsTreeExpanded)
						{
							DrawFieldWidget(dynamicFields[j]);
						}
					}

					ImGui::EndTable();
				}
				ImGui::PopID();
			}
		}

		// Certain macro value has been changed, reload dynamic fields
		if (material->IsDynamicFieldCacheAvailable())
		{
			material->ReloadShaderDataAndApplyDynamicFields();
		}
	}

	void MaterialInspector::DrawFieldWidget(const Ref<DynamicUniformFieldBase>& field) const
	{
		ImGui::TableNextColumn();

		const char* name = field->Name.c_str();
		// Push name as ID
		ImGui::PushID(name);
		{
			// Uniform name
			const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_SpanAvailWidth;
			ImGui::TreeNodeEx(name, flags);
			// Switch to the right column
			ImGui::TableNextColumn();
			// Align widget width to the right side
			ImGui::SetNextItemWidth(-1.0f);
			// Draw widget
			field->Draw();
		}
		ImGui::PopID();
	}

}

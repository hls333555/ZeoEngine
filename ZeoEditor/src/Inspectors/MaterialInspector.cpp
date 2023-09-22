#include "ZEpch.h"
#include "Inspectors/MaterialInspector.h"

#include <imgui_internal.h>

#include "Engine/GameFramework/Components.h"
#include "Widgets/DynamicUniformFieldWidgets.h"

namespace ZeoEngine {

	namespace Utils	{

		Scope<DynamicUniformFieldWidgetBase> ConstructDynamicUniformFieldWidget(const Ref<DynamicUniformFieldBase>& field)
		{
			switch (field->GetFieldType())
			{
				case DynamicUniformFieldType::Bool:
				case DynamicUniformFieldType::BoolMacro:
					return CreateScope<DynamicUniformBoolFieldWidget>(static_cast<DynamicUniformBoolFieldBase*>(field.get()));
				case DynamicUniformFieldType::Int:
					return CreateScope<DynamicUniformScalarNFieldWidget<I32>>(static_cast<DynamicUniformScalarNField<I32>*>(field.get()));
				case DynamicUniformFieldType::IntMacro:
					return CreateScope<DynamicUniformScalarNMacroFieldWidget>(static_cast<DynamicUniformScalarNMacroField*>(field.get()));
				case DynamicUniformFieldType::Float:
					return CreateScope<DynamicUniformScalarNFieldWidget<float>>(static_cast<DynamicUniformScalarNField<float>*>(field.get()));
				case DynamicUniformFieldType::Vec2:
					return CreateScope<DynamicUniformScalarNFieldWidget<Vec2, 2, float>>(static_cast<DynamicUniformScalarNField<Vec2, 2, float>*>(field.get()));
				case DynamicUniformFieldType::Vec3:
					return CreateScope<DynamicUniformScalarNFieldWidget<Vec3, 3, float>>(static_cast<DynamicUniformScalarNField<Vec3, 3, float>*>(field.get()));
				case DynamicUniformFieldType::Color:
					return CreateScope<DynamicUniformColorFieldWidget>(static_cast<DynamicUniformColorField*>(field.get()));
				case DynamicUniformFieldType::Texture2D:
					return CreateScope<DynamicUniformTexture2DFieldWidget>(static_cast<DynamicUniformTexture2DField*>(field.get()));
			}

			ZE_CORE_ASSERT(false);
			return nullptr;
		}
		
	}

	MaterialInspector::MaterialInspector(AssetPreviewWorldBase* world, U32 compID)
		: AssetInspector(world, compID)
	{
	}

	MaterialInspector::~MaterialInspector() = default;

	void MaterialInspector::ProcessDraw(Entity entity)
	{
		ImGui::Separator();

		const auto& materialComp = entity.GetComponent<MaterialDetailComponent>();
		const auto& material = materialComp.LoadedMaterial;
		const AssetHandle materialAsset = material->GetHandle();
		if (materialAsset != m_CurrentMaterialAsset)
		{
			if (const auto currentMaterial = AssetLibrary::LoadAsset<Material>(m_CurrentMaterialAsset))
			{
				currentMaterial->m_OnMaterialInitialized.disconnect(this);
			}
			material->m_OnMaterialInitialized.connect<&MaterialInspector::ClearFieldWidgets>(this);
			
			ClearFieldWidgets();
			m_CurrentMaterialAsset = materialAsset;
		}

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

	void MaterialInspector::DrawFieldWidget(const Ref<DynamicUniformFieldBase>& field)
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
			DrawDynamicUniformFieldWidget(field);
		}
		ImGui::PopID();
	}

	void MaterialInspector::DrawDynamicUniformFieldWidget(const Ref<DynamicUniformFieldBase>& field)
	{
		const U32 aggregatedID = ImGui::GetCurrentWindow()->GetID(field->Name.c_str());
		if (m_DynamicUniformFieldWidgets.find(aggregatedID) != m_DynamicUniformFieldWidgets.cend())
		{
			if (m_DynamicUniformFieldWidgets[aggregatedID])
			{
				m_DynamicUniformFieldWidgets[aggregatedID]->Draw();
			}
		}
		else
		{
			m_DynamicUniformFieldWidgets[aggregatedID] = Utils::ConstructDynamicUniformFieldWidget(field);
		}
	}

	void MaterialInspector::ClearFieldWidgets()
	{
		m_DynamicUniformFieldWidgets.clear();
	}

}

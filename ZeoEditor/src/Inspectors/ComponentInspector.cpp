#include "Inspectors/ComponentInspector.h"

#include <IconsFontAwesome5.h>

#include "Engine/Utils/ReflectionUtils.h"
#include "Inspectors/FieldWidget.h"
#include "Inspectors/ComponentFieldInstance.h"

namespace ZeoEngine {

	ComponentInspector::ComponentInspector(U32 compID)
		: m_ComponentID(compID)
	{
		PreprocessComponent();
	}

	ComponentInspector::~ComponentInspector() = default;

	void ComponentInspector::Draw(Entity entity)
	{
		if (!entity.HasComponentByID(m_ComponentID)) return;

		m_bWillRemove = false;

		const auto compType = entt::resolve(m_ComponentID);
		// Push component ID
		ImGui::PushID(m_ComponentID);
		{
			auto compInstance = entity.GetComponentByID(m_ComponentID);
			const char* compDisplayName = ReflectionUtils::GetComponentDisplayNameFull(m_ComponentID);

			bool bShouldDisplayCompHeader = !ReflectionUtils::DoesPropertyExist(Reflection::HideComponentHeader, compType);
			bool bIsCompHeaderExpanded = true;
			// Display component header
			if (bShouldDisplayCompHeader)
			{
				// Get available content region before adding CollapsingHeader as it will occupy space
				const ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();
				const float lineHeight = ImGui::GetFrameHeight();
				// Component collapsing header
				bIsCompHeaderExpanded = ImGui::CollapsingHeader(compDisplayName, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap);
				// Header tooltip
				if (const auto tooltip = ReflectionUtils::GetPropertyValue<const char*>(Reflection::Tooltip, compType))
				{
					ImGui::SetTooltipWithPadding(*tooltip);
				}

				ImGui::SameLine(contentRegionAvailable.x - lineHeight + ImGui::GetFramePadding().x);

				// Component settings button
				{
					if (ImGui::Button("...", { lineHeight, lineHeight }))
					{
						ImGui::OpenPopup("ComponentSettings");
					}

					if (ImGui::BeginPopupWithPadding("ComponentSettings"))
					{
						// Inherent components can never be removed
						bool bIsInherentComp = ReflectionUtils::DoesPropertyExist(Reflection::Inherent, compType);
						if (ImGui::MenuItem(ICON_FA_MINUS_CIRCLE "  Remove Component", nullptr, false, !bIsInherentComp))
						{
							m_bWillRemove = true;
						}

						ImGui::EndPopup();
					}
				}
			}
			if (bIsCompHeaderExpanded)
			{
				// Iterate all categories
				for (const auto& [category, fieldIDs] : m_PreprocessedFields)
				{
					bool bShouldDisplayCategoryTree = false;
					std::vector<entt::meta_data> visibleFields;
					// Do not show TreeNode if none of these fields will show or category is not set
					for (const auto fieldID : fieldIDs)
					{
						entt::meta_data data = compType.data(fieldID);
						if (!ShouldHideField(data, compInstance))
						{
							bShouldDisplayCategoryTree = !category.empty();
							visibleFields.push_back(data);
						}
					}
					bool bIsCategoryTreeExpanded = true;
					if (bShouldDisplayCategoryTree)
					{
						ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Selected | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_NoTreePushOnOpen/* Prevent indent of next row, which will affect column. */;
						// Field category tree
						bIsCategoryTreeExpanded = ImGui::TreeNodeEx(category.c_str(), flags);
					}
					if (bIsCategoryTreeExpanded)
					{
						const auto backupID = ImGui::GetItemID();
						// Sync table column separator
						ImGui::PushOverrideID(GetTableID());
						if (ImGui::BeginTable("", 2, ImGuiTableFlags_Resizable))
						{
							ImGui::TableNextColumn();

							ImGui::PushOverrideID(backupID);
							{
								// Iterate all visible fields
								for (const auto data : visibleFields)
								{
									// Push field ID
									ImGui::PushID(data.id());
									{
										// Draw widget based on data type
										DrawFieldWidget(data, entity);
									}
									ImGui::PopID();
								}

								DrawExtraFieldWidgets(entity);
							}
							ImGui::PopID();

							ImGui::EndTable();
						}
						ImGui::PopID();
					}
				}
			}
		}
		ImGui::PopID();
	}

	void ComponentInspector::PreprocessComponent()
	{
		const auto type = entt::resolve(m_ComponentID);
		ReflectionUtils::ForEachFieldInComponent(type, [this](entt::meta_data data)
		{
			PreprocessField(data);
		});
	}

	void ComponentInspector::PreprocessField(entt::meta_data data)
	{
		const auto categoryName = ReflectionUtils::GetPropertyValue<const char*>(Reflection::Category, data);
		const char* category = categoryName ? *categoryName : "";
		// Reverse data display order and categorize them
		m_PreprocessedFields[category].push_front(data.id());
	}

	bool ComponentInspector::ShouldHideField(entt::meta_data data, entt::meta_any& instance) const
	{
		auto bIsHiddenInEditor = ReflectionUtils::DoesPropertyExist(Reflection::HiddenInEditor, data);
		if (bIsHiddenInEditor) return true;

		auto hideCondition = ReflectionUtils::GetPropertyValue<HideConditionFunc>(Reflection::HideCondition, data);
		// HideCondition property is not set, show this data normally
		if (!hideCondition) return false;

		return (*hideCondition)(instance.try_cast<IComponent>());
	}

	void ComponentInspector::DrawFieldWidget(entt::meta_data data, Entity entity)
	{
		const U32 aggregatedID = ImGui::GetCurrentWindow()->GetID(data.id());
		if (m_FieldWidgets.find(aggregatedID) != m_FieldWidgets.cend())
		{
			if (m_FieldWidgets[aggregatedID])
			{
				m_FieldWidgets[aggregatedID]->Draw();
			}
		}
		else
		{
			const char* name = ReflectionUtils::GetMetaObjectName(data);
			const auto type = ReflectionUtils::MetaTypeToFieldType(data.type());
			auto fieldInstance = CreateRef<ComponentFieldInstance>(type, data, entity, m_ComponentID);
			m_FieldWidgets[aggregatedID] = Utils::ConstructFieldWidget<ComponentFieldInstance>(type, aggregatedID, std::move(fieldInstance));
		}
	}

}

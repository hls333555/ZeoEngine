#include "Inspectors/ComponentInspector.h"

#include <IconsFontAwesome5.h>

#include "Inspectors/DataWidget.h"
#include "Engine/Utils/ReflectionUtils.h"

namespace ZeoEngine {

	ComponentInspector::ComponentInspector(U32 compId)
		: m_ComponentId(compId)
	{
		PreprocessComponent();
	}

	void ComponentInspector::Draw(Entity entity)
	{
		m_bWillRemove = false;

		const auto compType = entt::resolve(m_ComponentId);
		// Push component id
		ImGui::PushID(m_ComponentId);
		{
			auto compInstance = entity.GetComponentById(m_ComponentId);
			const char* compDisplayName = ReflectionUtils::GetComponentDisplayNameFull(m_ComponentId);

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
				Utils::ShowPropertyTooltip(compType);

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
				for (const auto& [category, dataIds] : m_PreprocessedDatas)
				{
					bool bShouldDisplayCategoryTree = false;
					std::vector<entt::meta_data> visibleDatas;
					// Do not show TreeNode if none of these datas will show or category is not set
					for (const auto dataId : dataIds)
					{
						entt::meta_data data = compType.data(dataId);
						if (!m_DataParser.ShouldHideData(data, compInstance))
						{
							bShouldDisplayCategoryTree = !category.empty();
							visibleDatas.push_back(data);
						}
					}
					bool bIsCategoryTreeExpanded = true;
					if (bShouldDisplayCategoryTree)
					{
						// Data category tree
						bIsCategoryTreeExpanded = ImGui::TreeNodeEx(category.c_str(),
							ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Selected | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_NoTreePushOnOpen/* Prevent indent of next row, which will affect column. */);
					}
					if (bIsCategoryTreeExpanded)
					{
						// We want column separator to keep synced across different entities...
						// so we pop component id...
						ImGui::PopID();
						// and pop entity id
						ImGui::PopID();
						if (ImGui::BeginTable("", 2, ImGuiTableFlags_Resizable))
						{
							m_TableID = ImGui::GetItemID();
							ImGui::TableNextColumn();
							// Re-push entity id
							ImGui::PushID(static_cast<U32>(entity));
							// Re-push component id
							ImGui::PushID(m_ComponentId);

							// Iterate all visible datas
							for (const auto data : visibleDatas)
							{
								// Push data id
								ImGui::PushID(data.id());
								{
									// Draw widget based on data type
									DrawDataWidget(data, compInstance);
								}
								ImGui::PopID();
							}

							// When EndTable() is called, the ID stack must be the same as when BeginTable() is called!
							ImGui::PopID();
							ImGui::PopID();
							ImGui::EndTable();
						}
						// Re-push entity id
						ImGui::PushID(static_cast<U32>(entity));
						// Re-push component id
						ImGui::PushID(m_ComponentId);
					}
				}
			}
		}
		ImGui::PopID();
	}

	void ComponentInspector::PreprocessComponent()
	{
		for (const auto data : entt::resolve(m_ComponentId).data())
		{
			PreprocessData(data);
		}
	}

	void ComponentInspector::PreprocessData(entt::meta_data data)
	{
		const auto categoryName = ReflectionUtils::GetPropertyValue<const char*>(Reflection::Category, data);
		const char* category = categoryName ? *categoryName : "";
		// Reverse data display order and categorize them
		m_PreprocessedDatas[category].push_front(data.id());
	}

	void ComponentInspector::DrawDataWidget(entt::meta_data data, entt::meta_any& compInstance)
	{
		const U32 aggregatedDataId = Utils::GetAggregatedDataID(data);
		if (m_DataWidgets.find(aggregatedDataId) != m_DataWidgets.cend())
		{
			if (m_DataWidgets[aggregatedDataId])
			{
				m_DataWidgets[aggregatedDataId]->Draw(compInstance, compInstance);
			}
		}
		else
		{
			DataSpec dataSpec{ data, compInstance, compInstance, false, false };
			m_DataWidgets[aggregatedDataId] = ConstructBasicDataWidget(dataSpec, data.type());
		}
	}

}

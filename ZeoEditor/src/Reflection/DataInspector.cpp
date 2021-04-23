#include "Reflection/DataInspector.h"

#include <IconsFontAwesome5.h>

#include "Reflection/DataWidget.h"

namespace ZeoEngine {

	DataInspector::DataInspector(DataInspectorPanel* context)
		:m_Context(context)
	{
	}

	void DataInspector::ProcessComponent(entt::meta_type compType, Entity entity)
	{
		auto compInstance = entity.GetComponentById(compType.info().hash());
		const auto compId = compType.info().hash();
		const char* compDisplayName = GetComponentDisplayNameFull(compId);

		bool bShouldDisplayCompHeader = !DoesPropExist(PropertyType::HideComponentHeader, compType);
		bool bIsCompHeaderExpanded = true;
		bool bWillRemoveType = false;
		// Display component header
		if (bShouldDisplayCompHeader)
		{
			// Get available content region before adding CollapsingHeader as it will occupy space
			const ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();
			const float lineHeight = ImGui::GetFrameHeight();
			// Component collapsing header
			bIsCompHeaderExpanded = ImGui::CollapsingHeader(compDisplayName, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap);
			// Header tooltip
			ShowPropertyTooltip(compType);

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
					bool bIsInherentComp = DoesPropExist(PropertyType::Inherent, compType);
					if (ImGui::MenuItem(ICON_FA_MINUS_CIRCLE "  Remove Component", nullptr, false, !bIsInherentComp))
					{
						bWillRemoveType = true;
					}

					ImGui::EndPopup();
				}
			}
		}
		// Preprocess datas if needed
		if (m_bIsPreprocessedDatasDirty)
		{
			ZE_CORE_TRACE("Sorting datas on '{0}' of '{1}'", compDisplayName, entity.GetEntityName());
			PreprocessComponent(compType);
		}
		if (bIsCompHeaderExpanded)
		{
			// Iterate all categories
			for (const auto& [category, dataIds] : m_PreprocessedDatas[compId])
			{
				bool bShouldDisplayCategoryTree = false;
				std::vector<entt::meta_data> visibleDatas;
				// Do not show TreeNode if none of these datas will show or category is not set
				for (const auto dataId : dataIds)
				{
					entt::meta_data data = compType.data(dataId);
					if (!m_DataParser.ShouldHideData(data, compInstance))
					{
						bShouldDisplayCategoryTree = category != "";
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
					// We want column seperator to keep synced across different entities...
					// so we pop component id...
					ImGui::PopID();
					// and pop entity id
					ImGui::PopID();
					if (ImGui::BeginTable("", 2, ImGuiTableFlags_Resizable))
					{
						ImGui::TableNextColumn();
						// Re-push entity id
						ImGui::PushID(static_cast<uint32_t>(entity));
						// Re-push component id
						ImGui::PushID(compId);

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
					ImGui::PushID(static_cast<uint32_t>(entity));
					// Re-push component id
					ImGui::PushID(compId);
				}
			}
		}

		if (bWillRemoveType)
		{
			entity.RemoveComponentById(compId);
			m_PreprocessedDatas.erase(compId);
		}
	}

	void DataInspector::PreprocessComponent(entt::meta_type compType)
	{
		for (const auto data : compType.data())
		{
			PreprocessData(compType, data);
		}
	}

	void DataInspector::OnDrawComponentsComplete()
	{
		m_bIsPreprocessedDatasDirty = false;
	}

	void DataInspector::OnSelectedEntityChanged()
	{
		MarkCachesDirty();
	}

	void DataInspector::PreprocessData(entt::meta_type compType, entt::meta_data data)
	{
		auto categoryName = GetPropValue<const char*>(PropertyType::Category, data);
		const char* category = categoryName ? *categoryName : "";
		// Reverse data display order and categorize them
		m_PreprocessedDatas[compType.info().hash()][category].push_front(data.id());
	}

	void DataInspector::MarkCachesDirty()
	{
		m_DataParser.ClearCache();
		m_DataWidgets.clear();
		m_PreprocessedDatas.clear();
		m_bIsPreprocessedDatasDirty = true;
	}

	void DataInspector::DrawDataWidget(entt::meta_data data, entt::meta_any& compInstance)
	{
		uint32_t aggregatedDataId = GetAggregatedDataID(data);
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

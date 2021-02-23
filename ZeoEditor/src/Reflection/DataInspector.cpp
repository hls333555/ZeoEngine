#include "Reflection/DataInspector.h"

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
		const auto compName = GetMetaObjectDisplayName(compType);

		bool bShouldDisplayCompHeader = !DoesPropExist(PropertyType::HideComponentHeader, compType);
		bool bIsCompHeaderExpanded = true;
		bool bWillRemoveType = false;
		// Display component header
		if (bShouldDisplayCompHeader)
		{
			// Get available content region before adding CollapsingHeader as it will occupy space
			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			// Component collapsing header
			bIsCompHeaderExpanded = ImGui::CollapsingHeader(*compName, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap);
			// Header tooltip
			ShowPropertyTooltip(compType);

			ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);

			// Component settings button
			{
				if (ImGui::Button("...", { lineHeight, lineHeight }))
				{
					ImGui::OpenPopup("ComponentSettings");
				}

				if (ImGui::BeginPopup("ComponentSettings"))
				{
					// Inherent components can never be removed
					bool bIsInherentComp = DoesPropExist(PropertyType::Inherent, compType);
					if (ImGui::MenuItem("Remove Component", nullptr, false, !bIsInherentComp))
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
			ZE_CORE_TRACE("Sorting datas on {0} of '{1}'", *compName, entity.GetEntityName());
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
					// TODO:
					if (!ShouldHideData(data, compInstance))
					{
						bShouldDisplayCategoryTree = category != "";
						visibleDatas.push_back(data);
					}
				}
				bool bIsCategoryTreeExpanded = true;
				if (bShouldDisplayCategoryTree)
				{
					// Data category tree
					bIsCategoryTreeExpanded = ImGui::TreeNodeEx(category.c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Selected | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_NoTreePushOnOpen/* Prevent indent of next row, which will affect column. */);
				}
				if (bIsCategoryTreeExpanded)
				{
					// We want column seperator to keep synced across different entities...
					// so we pop component id...
					ImGui::PopID();
					// and pop entity id
					ImGui::PopID();
					if (ImGui::BeginTable("", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV))
					{
						ImGui::TableNextColumn();
						// Re-push entity id
						ImGui::PushID(static_cast<uint32_t>(entity));
						// Re-push component id
						ImGui::PushID(compId);

						ImGui::AlignTextToFramePadding();
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

	bool DataInspector::ShouldHideData(entt::meta_data data, const entt::meta_any& instance)
	{
		auto bIsHiddenInEditor = DoesPropExist(PropertyType::HiddenInEditor, data);
		if (bIsHiddenInEditor) return true;

		auto hideCondition = GetPropValue<const char*>(PropertyType::HideCondition, data);
		// HideCondition property is not set, show this data normally
		if (!hideCondition) return false;

		std::string hideConditionStr{ *hideCondition };
		std::optional<bool> result;
		// TODO: Add more operators
		ParseHideCondition(data, instance, hideConditionStr, "==", result);
		ParseHideCondition(data, instance, hideConditionStr, "!=", result);

		return *result;
	}

	void DataInspector::ParseHideCondition(entt::meta_data data, const entt::meta_any& instance, const std::string& hideConditionStr, const char* token, std::optional<bool>& result)
	{
		// Map from id to HideCondition key-value pair
		static std::unordered_map<uint32_t, std::pair<std::string, std::string>> hideConditionBuffers;

		// The string has been successfully parsed already, just return
		if (result) return;

		auto tokenPos = hideConditionStr.find(token);
		if (tokenPos == std::string::npos) return;

		std::string keyStr, valueStr;
		auto id = GetAggregatedDataID(data);
		if (hideConditionBuffers.find(id) != hideConditionBuffers.end())
		{
			keyStr = hideConditionBuffers[id].first;
			valueStr = hideConditionBuffers[id].second;
		}
		else
		{
			keyStr = hideConditionStr.substr(0, tokenPos);
			// Erase tail blanks
			keyStr.erase(keyStr.find_last_not_of(" ") + 1);

			valueStr = hideConditionStr.substr(tokenPos + 2, hideConditionStr.size() - 1);
			// Erase head blanks
			valueStr.erase(0, valueStr.find_first_not_of(" "));
			// Extract enum value if necessary (e.g. SceneCamera::ProjectionType::Perspective -> Perspective)
			auto valuePos = valueStr.rfind("::");
			if (valuePos != std::string::npos)
			{
				valueStr.erase(0, valuePos + 2);
			}

			hideConditionBuffers[id].first = keyStr;
			hideConditionBuffers[id].second = valueStr;
		}

		auto keyData = entt::resolve(instance.type().info().hash()).data(entt::hashed_string::value(keyStr.c_str()));
		auto keyDataValue = keyData.get(instance);

		// Bool
		{
			if (valueStr.find("true") != std::string::npos || valueStr.find("True") != std::string::npos)
			{
				if (token == "==")
				{
					result = keyDataValue.cast<bool>();
					return;
				}
				if (token == "!=")
				{
					result = !keyDataValue.cast<bool>();
					return;
				}
			}
			if (valueStr.find("false") != std::string::npos || valueStr.find("False") != std::string::npos)
			{
				if (token == "==")
				{
					result = !keyDataValue.cast<bool>();
					return;
				}
				if (token == "!=")
				{
					result = keyDataValue.cast<bool>();
					return;
				}
			}
		}

		// Enum
		{
			auto keyDataType = keyData.type();
			auto valueToCompare = keyDataType.data(entt::hashed_string::value(valueStr.c_str())).get({});
			if (token == "==")
			{
				result = keyDataValue == valueToCompare;
				return;
			}
			if (token == "!=")
			{
				result = keyDataValue != valueToCompare;
				return;
			}
		}
	}

	void DataInspector::MarkCachesDirty()
	{
		m_DataWidgets.clear();
		m_PreprocessedDatas.clear();
		m_bIsPreprocessedDatasDirty = true;
	}

	void DataInspector::DrawDataWidget(entt::meta_data data, const entt::meta_any& compInstance)
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
			m_DataWidgets[aggregatedDataId] = ConstructBasicDataWidget(dataSpec, m_Context);
		}
	}

}

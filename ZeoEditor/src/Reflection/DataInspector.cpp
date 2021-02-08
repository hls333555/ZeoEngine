#include "Reflection/DataInspector.h"

#include <misc/cpp/imgui_stdlib.h>

#include "Engine/Core/KeyCodes.h"
#include "Panels/DataInspectorPanel.h"
#include "Engine/Renderer/Texture.h"
#include "Engine/Utils/PlatformUtils.h"
#include "Engine/GameFramework/Components.h"
#include "Dockspaces/EditorDockspace.h"
#include "Utils/EditorUtils.h"

namespace ZeoEngine {

	DataInspector::DataInspector(DataInspectorPanel* context)
		:m_Context(context)
	{
	}

	static ImGuiTreeNodeFlags DefaultDataFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet;
	static ImGuiTreeNodeFlags EmptyContainerDataFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;
	static ImGuiTreeNodeFlags ContainerDataFlags = ImGuiTreeNodeFlags_DefaultOpen;
	static ImGuiTreeNodeFlags NestedDataFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth;;

	void DataInspector::ProcessComponent(entt::meta_type compType, Entity entity)
	{
		auto compInstance = entity.GetComponentById(compType.info().hash());
		// TODO:
		//m_DataCallbackInfo.ComponentInstance = compInstance;
		const auto compId = compType.info().hash();
		const auto compName = GetMetaObjectDisplayName(compType);

		bool bShouldDisplayCompHeader = compId != entt::type_hash<CoreComponent>::value();
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

								// TODO:
								//bool bIsNestedClass = DoesPropExist(PropertyType::NestedClass, dataType);
								//if (bIsNestedClass)
								//{
								//	flags = NestedDataFlags;
								//}
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

	uint32_t DataInspector::GetAggregatedDataID(entt::meta_data data)
	{
		return ImGui::GetCurrentWindow()->GetID(data.id());
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

	entt::meta_sequence_container::iterator DataInspector::InsertDefaultValueForSeq(entt::meta_data data, entt::meta_sequence_container& seqView, entt::meta_sequence_container::iterator it)
	{
		// "0" value works for "all" types because we have registered their conversion functions
		auto [retIt, res] = seqView.insert(it, 0);
		if (res)
		{
			//InvokePostDataValueEditChangeCallback(data, {});
			return retIt;
		}
		else
		{
			// For special types like user-defined enums, we have to invoke a function instead
			auto defaultValue = CreateTypeDefaultValue(seqView.value_type());
			auto [retIt, res] = seqView.insert(it, defaultValue);
			if (res)
			{
				//InvokePostDataValueEditChangeCallback(data, {});
				return retIt;
			}
			else
			{
				auto dataName = GetMetaObjectDisplayName(data);
				ZE_CORE_ASSERT(false, "Failed to insert with data: '{0}'!", *dataName);
			}
		}
		return {};
	}

	entt::meta_sequence_container::iterator DataInspector::EraseValueForSeq(entt::meta_data data, entt::meta_sequence_container& seqView, entt::meta_sequence_container::iterator it)
	{
		auto [retIt, res] = seqView.erase(it);
		if (res)
		{
			//InvokePostDataValueEditChangeCallback(data, {});
		}
		else
		{
			auto dataName = GetMetaObjectDisplayName(data);
			ZE_CORE_ERROR("Failed to erase with data: {0}!", *dataName);
		}
		return retIt;
	}

	void DataInspector::DrawContainerOperationWidget(entt::meta_data data, entt::meta_any& instance)
	{
		auto seqView = data.get(instance).as_sequence_container();

		ImGui::Text("%d elements", seqView.size());
		ImGui::SameLine();
		if (ImGui::BeginCombo("##ContainerOperation", nullptr, ImGuiComboFlags_NoPreview))
		{
			if (ImGui::Selectable("Add"))
			{
				InsertDefaultValueForSeq(data, seqView, seqView.end());
			}
			if (ImGui::Selectable("Clear"))
			{
				if (seqView.size() > 0 && seqView.clear())
				{
					// TODO:
					//InvokePostDataValueEditChangeCallback(data, {});
				}
			}

			ImGui::EndCombo();
		}
	}

	void DataInspector::DrawContainerElementOperationWidget(entt::meta_data data, entt::meta_sequence_container& seqView, entt::meta_sequence_container::iterator& it)
	{
		if (ImGui::BeginCombo("##ContainerElementOperation", nullptr, ImGuiComboFlags_NoPreview))
		{
			if (ImGui::Selectable("Insert"))
			{
				it = InsertDefaultValueForSeq(data, seqView, it);
			}
			if (ImGui::Selectable("Erase"))
			{
				it = EraseValueForSeq(data, seqView, it);
			}

			ImGui::EndCombo();
		}
	}

	void DataInspector::EvaluateSequenceContainerData(entt::meta_data data, entt::meta_any& instance)
	{
		auto seqView = data.get(instance).as_sequence_container();
		const auto type = seqView.value_type();
		uint32_t i = 0;
		for (auto it = seqView.begin(); it != seqView.end();)
		{
			auto element = *it;
			bool bIsNestedClass = DoesPropExist(PropertyType::NestedClass, type);
			ImGuiTreeNodeFlags flags = bIsNestedClass ? NestedDataFlags : DefaultDataFlags;
			// Data index
			bool bIsTreeExpanded = ImGui::TreeNodeEx(std::to_string(i).c_str(), flags);
			// Switch to the right column
			ImGui::TableNextColumn();
			// Push data index as id
			ImGui::PushID(i);
			{
				if (bIsNestedClass)
				{
					// Insert and erase buttons
					DrawContainerElementOperationWidget(data, seqView, it);
					// Switch to the next row
					ImGui::TableNextColumn();
					if (bIsTreeExpanded)
					{
						EvaluateSubData(data, element, true);

						ImGui::TreePop();
					}
				}
				else
				{
					// Make sure element widget + dropdown button can reach desired size
					ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();
					float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
					ImGui::SetNextItemWidth(contentRegionAvailable.x - lineHeight);
					EvaluateData(data, element, true);
					ImGui::SameLine();
					// Insert and erase buttons
					DrawContainerElementOperationWidget(data, seqView, it);
					// Switch to the next row
					ImGui::TableNextColumn();
				}
			}
			ImGui::PopID();

			if (it != seqView.end())
			{
				++it, ++i;
			}
		}
	}

	void DataInspector::EvaluateAssociativeContainerData(entt::meta_data data, entt::meta_any& instance)
	{
		
	}

	void DataInspector::EvaluateSubData(entt::meta_data data, entt::meta_any& instance, bool bIsSeqContainer)
	{
		const auto type = bIsSeqContainer ? instance.type() : data.type();
		auto subInstance = bIsSeqContainer ? instance : data.get(instance);
		// Clear before iterating subdatas
		m_DataCallbackInfo.ResetForSubData();
		// TODO: Reverse subdata order
		for (auto subData : type.data())
		{
			if (ShouldHideData(subData, subInstance)) continue;

			auto subDataName = GetMetaObjectDisplayName(subData);
			bool bIsNestedClass = DoesPropExist(PropertyType::NestedClass, subData.type());
			ImGuiTreeNodeFlags flags = bIsNestedClass ? NestedDataFlags : DefaultDataFlags;
			// Subdata name
			bool bIsTreeExpanded = ImGui::TreeNodeEx(*subDataName, flags);
			// Subdata tooltip
			ShowPropertyTooltip(subData);
			// Switch to the right column
			ImGui::TableNextColumn();
			if (bIsNestedClass)
			{
				// Switch to the next row
				ImGui::TableNextColumn();
			}
			if (bIsTreeExpanded)
			{
				// Push subdata id
				ImGui::PushID(subData.id());
				{
					if (bIsNestedClass)
					{
						EvaluateSubData(subData, subInstance, false);

						ImGui::TreePop();
					}
					else
					{
						// Align width to the right side
						ImGui::SetNextItemWidth(-1.0f);
						EvaluateData(subData, subInstance, false, true);
						// Switch to the next row
						ImGui::TableNextColumn();
					}
				}
				ImGui::PopID();
			}
		}
		// We must set subInstance value back to instance
		SetDataValue(data, instance, subInstance);
		if (m_DataCallbackInfo.ChangedSubData)
		{
			InvokePostDataValueEditChangeCallback(m_DataCallbackInfo.ChangedSubData, m_DataCallbackInfo.ChangedSubDataOldValue);
		}
	}

	// NOTE: Column is not switched inside this method and you should switch to the next row after this call
	void DataInspector::EvaluateData(entt::meta_data data, entt::meta_any& compInstance, bool bIsSeqContainer, bool bIsSubData)
	{
		const auto type = bIsSeqContainer ? compInstance.type() : data.type();
		if (type.is_integral())
		{
			EvaluateIntegralData(data, compInstance, bIsSeqContainer, bIsSubData);
		}
		else if (type.is_floating_point())
		{
			EvaluateFloatingPointData(data, compInstance, bIsSeqContainer, bIsSubData);
		}
		else if (type.is_enum())
		{
			ProcessEnumData(data, compInstance, bIsSeqContainer, bIsSubData);
		}
		else
		{
			EvaluateOtherData(data, compInstance, bIsSeqContainer, bIsSubData);
		}
	}

	void DataInspector::DrawDataWidget(entt::meta_data data, const entt::meta_any& compInstance)
	{
		uint32_t aggregatedDataId = GetAggregatedDataID(data);
		if (m_DataWidgets.find(aggregatedDataId) != m_DataWidgets.cend())
		{
			m_DataWidgets[aggregatedDataId]->Draw(compInstance);
		}
		else
		{
			DataSpec dataSpec{ data, compInstance };
			switch (dataSpec.Evaluate())
			{
			case BasicDataType::SEQCON:
				m_DataWidgets[aggregatedDataId] = CreateRef<SequenceContainerWidget>(dataSpec);
				break;
			case BasicDataType::ASSCON:
				m_DataWidgets[aggregatedDataId] = CreateRef<AssociativeContainerWidget>(dataSpec);
				break;
			case BasicDataType::BOOL:
				m_DataWidgets[aggregatedDataId] = CreateRef<BoolDataWidget>(dataSpec);
				break;
			case BasicDataType::I8:
				m_DataWidgets[aggregatedDataId] = CreateRef<ScalarNDataWidget<int8_t>>(dataSpec, ImGuiDataType_S8, static_cast<int8_t>(INT8_MIN), static_cast<int8_t>(INT8_MAX), "%hhd");
				break;
			case BasicDataType::I32:
				m_DataWidgets[aggregatedDataId] = CreateRef<ScalarNDataWidget<int32_t>>(dataSpec, ImGuiDataType_S32, INT32_MIN, INT32_MAX, "%d");
				break;
			case BasicDataType::I64:
				m_DataWidgets[aggregatedDataId] = CreateRef<ScalarNDataWidget<int64_t>>(dataSpec, ImGuiDataType_S64, INT64_MIN, INT64_MAX, "%lld");
				break;
			case BasicDataType::UI8:
				m_DataWidgets[aggregatedDataId] = CreateRef<ScalarNDataWidget<uint8_t>>(dataSpec, ImGuiDataType_U8, 0ui8, UINT8_MAX, "%hhu");
				break;
			case BasicDataType::UI32:
				m_DataWidgets[aggregatedDataId] = CreateRef<ScalarNDataWidget<uint32_t>>(dataSpec, ImGuiDataType_U32, 0ui32, UINT32_MAX, "%u");
				break;
			case BasicDataType::UI64:
				m_DataWidgets[aggregatedDataId] = CreateRef<ScalarNDataWidget<uint64_t>>(dataSpec, ImGuiDataType_U64, 0ui64, UINT64_MAX, "%llu");
				break;
			case BasicDataType::FLOAT:
				m_DataWidgets[aggregatedDataId] = CreateRef<ScalarNDataWidget<float>>(dataSpec, ImGuiDataType_Float, -FLT_MAX, FLT_MAX, "%.2f");
				break;
			case BasicDataType::DOUBLE:
				m_DataWidgets[aggregatedDataId] = CreateRef<ScalarNDataWidget<double>>(dataSpec, ImGuiDataType_Double, -DBL_MAX, DBL_MAX, "%.3lf");
				break;
			case BasicDataType::ENUM:
				m_DataWidgets[aggregatedDataId] = CreateRef<EnumDataWidget>(dataSpec);
				break;
			case BasicDataType::STRING:
				m_DataWidgets[aggregatedDataId] = CreateRef<StringDataWidget>(dataSpec);
				break;
			case BasicDataType::VEC2:
				m_DataWidgets[aggregatedDataId] = CreateRef<ScalarNDataWidget<glm::vec2, 2, float>>(dataSpec, ImGuiDataType_Float, -FLT_MAX, FLT_MAX, "%.2f");
				break;
			case BasicDataType::VEC3:
				m_DataWidgets[aggregatedDataId] = CreateRef<ScalarNDataWidget<glm::vec3, 3, float>>(dataSpec, ImGuiDataType_Float, -FLT_MAX, FLT_MAX, "%.2f");
				break;
			case BasicDataType::VEC4:
				m_DataWidgets[aggregatedDataId] = CreateRef<ColorDataWidget>(dataSpec);
				break;
			case BasicDataType::TEXTURE:
				m_DataWidgets[aggregatedDataId] = CreateRef<Texture2DDataWidget>(dataSpec);
				break;
			case BasicDataType::PARTICLE:
				
				break;
			default:
				break;
			}
		}
	}

	void DataInspector::EvaluateIntegralData(entt::meta_data data, entt::meta_any& instance, bool bIsSeqContainer, bool bIsSubData)
	{
		const auto type = bIsSeqContainer ? instance.type() : data.type();
		if (IsTypeEqual<bool>(type))
		{
			ProcessBoolData(data, instance, bIsSeqContainer, bIsSubData);
		}
		else if (IsTypeEqual<int8_t>(type))
		{
			ProcessScalarNData<int8_t>(data, instance, bIsSeqContainer, bIsSubData, ImGuiDataType_S8, static_cast<int8_t>(INT8_MIN), static_cast<int8_t>(INT8_MAX), "%hhd");
		}
		else if (IsTypeEqual<int32_t>(type))
		{
			ProcessScalarNData<int32_t>(data, instance, bIsSeqContainer, bIsSubData, ImGuiDataType_S32, INT32_MIN, INT32_MAX, "%d");
		}
		else if (IsTypeEqual<int64_t>(type))
		{
			ProcessScalarNData<int64_t>(data, instance, bIsSeqContainer, bIsSubData, ImGuiDataType_S64, INT64_MIN, INT64_MAX, "%lld");
		}
		else if (IsTypeEqual<uint8_t>(type))
		{
			ProcessScalarNData<uint8_t>(data, instance, bIsSeqContainer, bIsSubData, ImGuiDataType_U8, 0ui8, UINT8_MAX, "%hhu");
		}
		else if (IsTypeEqual<uint32_t>(type))
		{
			ProcessScalarNData<uint32_t>(data, instance, bIsSeqContainer, bIsSubData, ImGuiDataType_U32, 0ui32, UINT32_MAX, "%u");
		}
		else if (IsTypeEqual<uint64_t>(type))
		{
			ProcessScalarNData<uint64_t>(data, instance, bIsSeqContainer, bIsSubData, ImGuiDataType_U64, 0ui64, UINT64_MAX, "%llu");
		}
	}

	void DataInspector::EvaluateFloatingPointData(entt::meta_data data, entt::meta_any& instance, bool bIsSeqContainer, bool bIsSubData)
	{
		const auto type = bIsSeqContainer ? instance.type() : data.type();
		if (IsTypeEqual<float>(type))
		{
			ProcessScalarNData<float>(data, instance, bIsSeqContainer, bIsSubData, ImGuiDataType_Float, -FLT_MAX, FLT_MAX, "%.2f");
		}
		else if (IsTypeEqual<double>(type))
		{
			ProcessScalarNData<double>(data, instance, bIsSeqContainer, bIsSubData, ImGuiDataType_Double, -DBL_MAX, DBL_MAX, "%.3lf");
		}
	}

	void DataInspector::EvaluateOtherData(entt::meta_data data, entt::meta_any& instance, bool bIsSeqContainer, bool bIsSubData)
	{
		const auto type = bIsSeqContainer ? instance.type() : data.type();
		if (IsTypeEqual<std::string>(type))
		{
			ProcessStringData(data, instance, bIsSeqContainer, bIsSubData);
		}
		else if (IsTypeEqual<glm::vec2>(type))
		{
			ProcessScalarNData<glm::vec2, 2, float>(data, instance, bIsSeqContainer, bIsSubData, ImGuiDataType_Float, -FLT_MAX, FLT_MAX, "%.2f");
		}
		else if (IsTypeEqual<glm::vec3>(type))
		{
			ProcessScalarNData<glm::vec3, 3, float>(data, instance, bIsSeqContainer, bIsSubData, ImGuiDataType_Float, -FLT_MAX, FLT_MAX, "%.2f");
		}
		else if (IsTypeEqual<glm::vec4>(type))
		{
			ProcessColorData(data, instance, bIsSeqContainer, bIsSubData);
		}
		else if (IsTypeEqual<Ref<Texture2D>>(type))
		{
			ProcessTexture2DData(data, instance, bIsSeqContainer, bIsSubData);
		}
		else if (IsTypeEqual<Ref<ParticleTemplate>>(type))
		{
			ProcessParticleTemplateData(data, instance, bIsSeqContainer, bIsSubData);
		}
	}

	void DataInspector::InvokeOnDataValueEditChangeCallback(entt::meta_data data, std::any oldValue)
	{
		ZE_TRACE("Value changed!");
		InternalInvokeOnDataValueEditChangeCallback(m_DataCallbackInfo.ComponentInstance.type(), m_DataCallbackInfo.ComponentInstance, data.id(), oldValue);
	}

	void DataInspector::InvokePostDataValueEditChangeCallback(entt::meta_data data, std::any oldValue)
	{
		ZE_TRACE("Value changed after edit!");
		InternalInvokePostDataValueEditChangeCallback(m_DataCallbackInfo.ComponentInstance.type(), m_DataCallbackInfo.ComponentInstance, data.id(), oldValue);
	}

	void DataInspector::ProcessBoolData(entt::meta_data data, entt::meta_any& instance, bool bIsSeqContainer, bool bIsSubData)
	{
		auto& boolRef = bIsSeqContainer/* ? instance.cast<bool>() : GetDataValueByRef<bool>(data, instance)*/;
		const bool bUseCopy = DoesPropExist(PropertyType::AsCopy, data);
		auto boolCopy = bUseCopy ? GetDataValue<bool>(data, instance) : false;
		auto& boolValue = bUseCopy ? boolCopy : boolRef;
		auto oldValue = boolValue;

		// NOTE: We cannot leave Checkbox's label empty
		if (ImGui::Checkbox("##Bool", &boolValue))
		{
			if (bUseCopy)
			{
				SetDataValue(data, instance, boolCopy);
			}

			if (bIsSubData)
			{
				m_DataCallbackInfo.ChangedSubData = data;
				m_DataCallbackInfo.ChangedSubDataOldValue = oldValue;
			}
			else
			{
				InvokePostDataValueEditChangeCallback(data, oldValue);
			}
		}
	}

	void DataInspector::ProcessEnumData(entt::meta_data data, entt::meta_any& instance, bool bIsSeqContainer, bool bIsSubData)
	{
		auto enumValue = bIsSeqContainer ? instance : data.get(instance);
		const char* currentValueName = GetEnumDisplayName(enumValue);

		// NOTE: We cannot leave ComboBox's label empty
		if (ImGui::BeginCombo("##Enum", currentValueName))
		{
			// TODO: Reverse enum display order
			// Iterate to display all enum values
			const auto& datas = bIsSeqContainer ? instance.type().data() : data.type().data();
			for (auto enumData : datas)
			{
				auto valueName = GetMetaObjectDisplayName(enumData);
				bool bIsSelected = ImGui::Selectable(*valueName);
				ShowPropertyTooltip(enumData);
				if (bIsSelected)
				{
					ImGui::SetItemDefaultFocus();
					auto newValue = enumData.get({});
					if (newValue != enumValue)
					{
						auto oldValue = enumValue;
						if (bIsSeqContainer)
						{
							SetEnumValueForSeq(instance, newValue);
						}
						else
						{
							SetDataValue(data, instance, newValue);
						}
						if (bIsSubData)
						{
							m_DataCallbackInfo.ChangedSubData = data;
							m_DataCallbackInfo.ChangedSubDataOldValue = oldValue;
						}
						else
						{
							InvokePostDataValueEditChangeCallback(data, oldValue);
						}
					}
				}
			}

			ImGui::EndCombo();
		}
	}

	void DataInspector::ProcessStringData(entt::meta_data data, entt::meta_any& instance, bool bIsSeqContainer, bool bIsSubData)
	{
		// Map from id to string cache plus a bool flag indicating if we are editing the text
		static std::unordered_map<uint32_t, std::pair<bool, std::string>> stringBuffers;
		auto& stringRef = bIsSeqContainer ? instance.cast<std::string>() : GetDataValueByRef<std::string>(data, instance);
		auto id = GetAggregatedDataID(data);
		const bool bUseCopy = DoesPropExist(PropertyType::AsCopy, data);
		std::string stringCopy = bUseCopy ? GetDataValue<std::string>(data, instance) : std::string();
		auto& stringValue = bUseCopy ? stringCopy : stringRef;
		auto oldValue = stringValue;

		// NOTE: We cannot leave InputBox's label empty
		ImGui::InputText("##String", stringBuffers[id].first ? &stringBuffers[id].second : &stringValue, ImGuiInputTextFlags_AutoSelectAll);
		if (bUseCopy && !stringBuffers[id].first)
		{
			SetDataValue(data, instance, std::move(stringCopy));
		}

		// Write changes to cache first
		if (ImGui::IsItemActivated())
		{
			stringBuffers[id].first = true;
			stringBuffers[id].second = stringValue;
		}

		bool bIsValueChanged = false;
		// Apply cache when user finishes editing
		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			bIsValueChanged = stringBuffers[id].second != stringValue;
			stringBuffers[id].first = false;
			if (bIsSeqContainer)
			{
				stringRef = std::move(stringBuffers[id].second);
			}
			else
			{
				SetDataValue(data, instance, std::move(stringBuffers[id].second));
			}
		}

		if (bIsValueChanged)
		{
			if (bIsSubData)
			{
				m_DataCallbackInfo.ChangedSubData = data;
				m_DataCallbackInfo.ChangedSubDataOldValue = oldValue;
			}
			else
			{
				InvokePostDataValueEditChangeCallback(data, oldValue);
			}
		}
	}

	void DataInspector::ProcessColorData(entt::meta_data data, entt::meta_any& instance, bool bIsSeqContainer, bool bIsSubData)
	{
		// Map from id to value cache plus a bool flag indicating if displayed value is retrieved from cache
		static std::unordered_map<uint32_t, std::pair<bool, glm::vec4>> vec4Buffers;
		auto& vec4Ref = bIsSeqContainer ? instance.cast<glm::vec4>() : GetDataValueByRef<glm::vec4>(data, instance);
		auto id = GetAggregatedDataID(data);
		const bool bUseCopy = DoesPropExist(PropertyType::AsCopy, data);
		glm::vec4 vec4Copy = bUseCopy ? GetDataValue<glm::vec4>(data, instance) : glm::vec4();
		auto& vec4Value = bUseCopy ? vec4Copy : vec4Ref;
		auto oldValue = vec4Value;

		bool bResult = ImGui::ColorEdit4("", vec4Buffers[id].first ? glm::value_ptr(vec4Buffers[id].second) : glm::value_ptr(vec4Value));
		if (bUseCopy && !vec4Buffers[id].first)
		{
			SetDataValue(data, instance, std::move(vec4Copy));
		}

		bool bIsValueChangedAfterEdit = false;
		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			bIsValueChangedAfterEdit = vec4Buffers[id].second != vec4Value;
			if (vec4Buffers[id].first)
			{
				// Apply cache when input box is inactive
				// Dragging will not go here
				if (bIsSeqContainer)
				{
					vec4Ref = std::move(vec4Buffers[id].second);
				}
				else
				{
					// For copied value if we tab into the 2nd or later component, we must update the copy first
					if (bUseCopy)
					{
						vec4Value = vec4Buffers[id].second;
					}
					SetDataValue(data, instance, std::move(vec4Buffers[id].second));
				}
			}
			vec4Buffers[id].first = false;
		}

		if (ImGui::IsItemActivated())
		{
			//  Update cache when this item is activated
			vec4Buffers[id].second = vec4Value;

			ImGuiContext* context = ImGui::GetCurrentContext();
			// Input box is activated by double clicking, CTRL-clicking or being tabbed in
			if (ImGui::IsMouseDoubleClicked(0) ||
				(context->IO.KeyCtrl && ImGui::IsMouseClicked(0)) ||
				context->NavJustTabbedId == context->ActiveId)
			{
				// Keep writing to cache as long as input box is active
				vec4Buffers[id].first = true;
			}
		}

		// Value changed during dragging
		if (bResult && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
		{
			if (bIsSubData)
			{
				m_DataCallbackInfo.ChangedSubData = data;
				m_DataCallbackInfo.ChangedSubDataOldValue = oldValue;
			}
			else
			{
				InvokeOnDataValueEditChangeCallback(data, oldValue);
			}
		}

		// Value changed after dragging or inputting
		if (bIsValueChangedAfterEdit)
		{
			if (bIsSubData)
			{
				m_DataCallbackInfo.ChangedSubData = data;
				m_DataCallbackInfo.ChangedSubDataOldValue = oldValue;
			}
			else
			{
				InvokePostDataValueEditChangeCallback(data, oldValue);
			}
		}
	}

	void DataInspector::ProcessTexture2DData(entt::meta_data data, entt::meta_any& instance, bool bIsSeqContainer, bool bIsSubData)
	{
		auto& texture2DRef = bIsSeqContainer ? instance.cast<Ref<Texture2D>>() : GetDataValueByRef<Ref<Texture2D>>(data, instance);
		const bool bUseCopy = DoesPropExist(PropertyType::AsCopy, data);
		Ref<Texture2D> texture2DCopy = bUseCopy ? GetDataValue<Ref<Texture2D>>(data, instance) : Ref<Texture2D>();
		const auto& texture2DValue = bUseCopy ? texture2DCopy : texture2DRef;
		auto oldValue = texture2DValue;

		Texture2DLibrary& library = Texture2DLibrary::Get();
		// Texture preview
		{
			auto backgroundTexture = library.Get("assets/textures/Checkerboard_Alpha.png");
			const float texturePreviewWidth = 75.0f;
			// Draw checkerboard texture as background first
			ImGui::GetWindowDrawList()->AddImage(backgroundTexture->GetTexture(),
				ImGui::GetCursorScreenPos(),
				{ ImGui::GetCursorScreenPos().x + texturePreviewWidth, ImGui::GetCursorScreenPos().y + texturePreviewWidth },
				{ 0.0f, 1.0f }, { 1.0f, 0.0f });
			// Draw our texture on top of that
			ImGui::Image(texture2DValue ? texture2DValue->GetTexture() : nullptr,
				{ texturePreviewWidth, texturePreviewWidth },
				{ 0.0f, 1.0f }, { 1.0f, 0.0f },
				texture2DValue ? ImVec4{ 1.0f, 1.0f, 1.0f, 1.0f } : ImVec4{ 1.0f, 1.0f, 1.0f, 0.0f });
			// Display texture info tooltip
			if (texture2DValue && ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Resolution: %dx%d\nHas alpha: %s", texture2DValue->GetWidth(), texture2DValue->GetHeight(), texture2DValue->HasAlpha() ? "true" : "false");
			}
		}

		ImGui::SameLine();
		if (bIsSeqContainer)
		{
			// Make sure browser widget + dropdown button can reach desired size
			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImGui::SetNextItemWidth(contentRegionAvailable.x - lineHeight);
		}
		else
		{
			// Align width to the right side
			ImGui::SetNextItemWidth(-1.0f);
		}

		// Texture browser
		if (ImGui::BeginCombo("##Texture2D", texture2DValue ? texture2DValue->GetFileName().c_str() : nullptr))
		{
			bool bIsValueChangedAfterEdit = false;
			// Pop up file browser to select a texture from disk
			if (ImGui::Selectable("Browse texture..."))
			{
				auto filePath = FileDialogs::OpenFile(AssetType::Texture);
				if (filePath)
				{
					// Add selected texture to the library
					Ref<Texture2D> loadedTexture = library.GetOrLoad(*filePath);
					bIsValueChangedAfterEdit = loadedTexture != texture2DValue;
					if (bIsValueChangedAfterEdit)
					{
						if (bIsSeqContainer)
						{
							texture2DRef = loadedTexture;
						}
						else
						{
							SetDataValue(data, instance, loadedTexture);
						}
					}
				}
			}
			ImGui::Separator();
			// List all loaded textures from Texture2DLibrary
			for (const auto& [path, texture] : library.GetTexturesMap())
			{
				ImGui::PushID(texture->GetPath().c_str());

				const float textureThumbnailWidth = 30.0f;
				bool bIsSelected = ImGui::Selectable("##TextureDropdownThumbnail", false, 0, ImVec2(0.0f, textureThumbnailWidth));
				// Display texture path tooltip for drop-down item
				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltip("%s", texture->GetPath().c_str());
				}
				ImGui::SameLine();
				// Draw texture thumbnail
				ImGui::Image(texture->GetTexture(),
					ImVec2(textureThumbnailWidth, textureThumbnailWidth),
					ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
				ImGui::SameLine();
				// Display texture name
				ImGui::Text(texture->GetFileName().c_str());
				if (bIsSelected)
				{
					bIsValueChangedAfterEdit = texture != texture2DValue;
					if (bIsValueChangedAfterEdit)
					{
						if (bIsSeqContainer)
						{
							texture2DRef = texture;
						}
						else
						{
							SetDataValue(data, instance, texture);
						}
					}
				}

				ImGui::PopID();
			}
			ImGui::EndCombo();

			if (bIsValueChangedAfterEdit)
			{
				if (bIsSubData)
				{
					m_DataCallbackInfo.ChangedSubData = data;
					m_DataCallbackInfo.ChangedSubDataOldValue = oldValue;
				}
				else
				{
					InvokePostDataValueEditChangeCallback(data, oldValue);
				}
			}
		}

		// Display texture path tooltip for current selection
		if (texture2DValue && ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("%s", texture2DValue->GetPath().c_str());
		}
	}

	void DataInspector::ProcessParticleTemplateData(entt::meta_data data, entt::meta_any& instance, bool bIsSeqContainer, bool bIsSubData)
	{
		auto& particleTemplateRef = bIsSeqContainer ? instance.cast<Ref<ParticleTemplate>>() : GetDataValueByRef<Ref<ParticleTemplate>>(data, instance);
		const bool bUseCopy = DoesPropExist(PropertyType::AsCopy, data);
		Ref<ParticleTemplate> particleTemplateCopy = bUseCopy ? GetDataValue<Ref<ParticleTemplate>>(data, instance) : Ref<ParticleTemplate>();
		const auto& particleTemplateValue = bUseCopy ? particleTemplateCopy : particleTemplateRef;
		auto oldValue = particleTemplateValue;

		ParticleLibrary& library = ParticleLibrary::Get();
		Texture2DLibrary& texture2DLib = Texture2DLibrary::Get();
		auto backgroundTexture = texture2DLib.Get("assets/textures/Checkerboard_Alpha.png");
		// Particle template preview
		{
			const float pTemplatePreviewWidth = 75.0f;
			// Draw checkerboard texture as background first
			ImGui::GetWindowDrawList()->AddImage(backgroundTexture->GetTexture(),
				ImGui::GetCursorScreenPos(),
				{ ImGui::GetCursorScreenPos().x + pTemplatePreviewWidth, ImGui::GetCursorScreenPos().y + pTemplatePreviewWidth },
				{ 0.0f, 1.0f }, { 1.0f, 0.0f });
			// Draw preview thumbnail on top of that
			auto thumbnailTexture = particleTemplateValue && particleTemplateValue->PreviewThumbnail ? particleTemplateValue->PreviewThumbnail : backgroundTexture;
			ImGui::Image(thumbnailTexture->GetTexture(),
				{ pTemplatePreviewWidth, pTemplatePreviewWidth },
				{ 0.0f, 1.0f }, { 1.0f, 0.0f },
				particleTemplateValue ? ImVec4{ 1.0f, 1.0f, 1.0f, 1.0f } : ImVec4{ 1.0f, 1.0f, 1.0f, 0.0f });
			// Double-click to open the particle editor only when particle template is valid
			if (particleTemplateValue && ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Double-click to open the particle editor");
				if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				{
					EditorDockspace* editor = m_Context->GetContext()->OpenEditor(EditorDockspaceType::Particle_Editor);
					editor->GetContextEntity().PatchComponent<ParticleSystemPreviewComponent>([&particleTemplateValue](auto& pspc)
					{
						pspc.SetTemplate(particleTemplateValue);
					});
				}
			}
		}

		ImGui::SameLine();
		if (bIsSeqContainer)
		{
			// Make sure browser widget + dropdown button can reach desired size
			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImGui::SetNextItemWidth(contentRegionAvailable.x - lineHeight);
		}
		else
		{
			// Align width to the right side
			ImGui::SetNextItemWidth(-1.0f);
		}

		// Particle template browser
		if (ImGui::BeginCombo("##ParticleTemplate", particleTemplateValue ? particleTemplateValue->GetName().c_str() : nullptr))
		{
			bool bIsValueChangedAfterEdit = false;
			// Pop up file browser to select a particle template from disk
			if (ImGui::Selectable("Browse particle template..."))
			{
				auto filePath = FileDialogs::OpenFile(AssetType::ParticleTemplate);
				if (filePath)
				{
					// Add selected particle template to the library
					Ref<ParticleTemplate> loadedTemplate = library.GetOrLoad(*filePath);
					bIsValueChangedAfterEdit = loadedTemplate != particleTemplateValue;
					if (bIsValueChangedAfterEdit)
					{
						if (bIsSeqContainer)
						{
							particleTemplateRef = loadedTemplate;
						}
						else
						{
							SetDataValue(data, instance, loadedTemplate);
						}
					}
				}
			}
			ImGui::Separator();
			// List all loaded templates from ParticleLibrary
			for (const auto& [path, pTemplate] : library.GetParticleTemplatesMap())
			{
				ImGui::PushID(pTemplate->GetPath().c_str());

				const float pTemplateThumbnailWidth = 30.0f;
				bool bIsSelected = ImGui::Selectable("##ParticleTemplateDropdownThumbnail", false, 0, ImVec2(0.0f, pTemplateThumbnailWidth));
				// Display particle template path tooltip for drop-down item
				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltip("%s", pTemplate->GetPath().c_str());
				}
				ImGui::SameLine();
				// Draw particle template thumbnail
				auto thumbnailTexture = pTemplate && pTemplate->PreviewThumbnail ? pTemplate->PreviewThumbnail : backgroundTexture;
				ImGui::Image(thumbnailTexture->GetTexture(),
					ImVec2(pTemplateThumbnailWidth, pTemplateThumbnailWidth),
					ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
				ImGui::SameLine();
				// Display particle template name
				ImGui::Text(pTemplate->GetName().c_str());
				if (bIsSelected)
				{
					bIsValueChangedAfterEdit = pTemplate != particleTemplateValue;
					if (bIsValueChangedAfterEdit)
					{
						if (bIsSeqContainer)
						{
							particleTemplateRef = pTemplate;
						}
						else
						{
							SetDataValue(data, instance, pTemplate);
						}
					}
				}

				ImGui::PopID();
			}
			ImGui::EndCombo();

			if (bIsValueChangedAfterEdit)
			{
				if (bIsSubData)
				{
					m_DataCallbackInfo.ChangedSubData = data;
					m_DataCallbackInfo.ChangedSubDataOldValue = oldValue;
				}
				else
				{
					InvokePostDataValueEditChangeCallback(data, oldValue);
				}
			}
		}

		// Display particle template path tooltip for current selection
		if (particleTemplateValue && ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("%s", particleTemplateValue->GetPath().c_str());
		}
	}

}

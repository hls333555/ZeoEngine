#pragma once

#include <map>

#include "Engine/Core/Log.h"
#include "Engine/GameFramework/Entity.h"
#include "Engine/Core/ReflectionHelper.h"
#include "Engine/ImGui/MyImGui.h"

namespace ZeoEngine {

	class DataInspectorPanel;

	class DataInspector
	{
	public:
		DataInspector(DataInspectorPanel* context);

		/** Returns true if this type is removed during processing. */
		bool ProcessType(entt::meta_type type, Entity entity);

		void MarkPreprocessedDatasClean();
		void MarkPreprocessedDatasDirty();

	private:
		template<typename T>
		void ShowPropertyTooltip(T metaObj)
		{
			if (ImGui::IsItemHovered())
			{
				auto tooltip = GetPropValue<const char*>(PropertyType::Tooltip, metaObj);
				if (tooltip)
				{
					ImGui::SetTooltip(*tooltip);
				}
			}
		}

		uint32_t GetUniqueDataID(entt::meta_data data);

		void PreprocessData(entt::meta_type type, entt::meta_data data);

		// NOTE: Do not pass entt::meta_handle around as it does not support copy
		bool ShouldHideData(entt::meta_data data, const entt::meta_any& instance);

		void EvaluateSequenceContainerData(entt::meta_data data, entt::meta_any& instance);
		void EvaluateAssociativeContainerData(entt::meta_data data, entt::meta_any& instance);
		void EvaluateNestedData(entt::meta_data data, entt::meta_any& instance, bool bIsSeqContainer);
		void EvaluateData(entt::meta_data data, entt::meta_any& instance, bool bIsSeqContainer);

		void EvaluateIntegralData(entt::meta_data data, entt::meta_any& instance, bool bIsSeqContainer);
		void EvaluateFloatingPointData(entt::meta_data data, entt::meta_any& instance, bool bIsSeqContainer);
		void EvaluateOtherData(entt::meta_data data, entt::meta_any& instance, bool bIsSeqContainer);

		void ProcessBoolData(entt::meta_data data, entt::meta_any& instance, bool bIsSeqContainer);
		template<typename T, uint32_t N = 1, typename CT = T>
		void ProcessScalarNData(entt::meta_data data, entt::meta_any& instance, bool bIsSeqContainer, ImGuiDataType scalarType, CT defaultMin, CT defaultMax, const char* format)
		{
			static_assert(N == 1 || N == 2 || N == 3, "N can only be 1, 2 or 3!");

			// Map from id to value cache plus a bool flag indicating if displayed value is retrieved from cache
			static std::unordered_map<uint32_t, std::pair<bool, T>> valueBuffers;
			auto& valueRef = bIsSeqContainer ? instance.cast<T>() : GetDataValueByRef<T>(data, instance);
			auto speed = GetPropValue<float>(PropertyType::DragSensitivity, data);
			auto min = GetPropValue<CT>(PropertyType::ClampMin, data);
			auto minValue = min.value_or(defaultMin);
			auto max = GetPropValue<CT>(PropertyType::ClampMax, data);
			auto maxValue = max.value_or(defaultMax);
			ImGuiSliderFlags clampMode = DoesPropExist(PropertyType::ClampOnlyDuringDragging, data) ? 0 : ImGuiSliderFlags_AlwaysClamp;
			auto id = GetUniqueDataID(data);
			const bool bUseCopy = DoesPropExist(PropertyType::AsCopy, data);
			// Copy the value
			T valueCopy = bUseCopy ? GetDataValue<T>(data, instance) : T();
			auto& value = bUseCopy ? valueCopy : valueRef;

			void* valuePtr = nullptr;
			void* cachedValuePtr = nullptr;
			if constexpr (N == 1) // C++17 constexpr if
			{
				valuePtr = &value;
				cachedValuePtr = &valueBuffers[id].second;
			}
			else
			{
				valuePtr = glm::value_ptr(value);
				cachedValuePtr = glm::value_ptr(valueBuffers[id].second);
			}
			// For dragging, the value is applied immediately
			// For editing, the value is applied after completion
			bool bResult = ImGui::DragScalarNEx("", scalarType, valueBuffers[id].first ? cachedValuePtr : valuePtr, N, speed.value_or(1.0f), &minValue, &maxValue, format, clampMode);
			if (bUseCopy && !valueBuffers[id].first)
			{
				SetDataValue(data, instance, std::move(valueCopy));
			}

			// For multi-component drag UI, tabbing will automatically switch between those components, so we must handle deactivation first after tabbing to the next component
			bool bIsValueChangedAfterEdit = false;
			if (ImGui::IsItemDeactivatedAfterEdit())
			{
				bIsValueChangedAfterEdit = valueBuffers[id].second != value;
				if (valueBuffers[id].first)
				{
					// Apply cache when input box is inactive
					// Dragging will not go here
					if (bIsSeqContainer)
					{
						valueRef = std::move(valueBuffers[id].second);
					}
					else
					{
						// For copied value if we tab into the 2nd or later component, we must update the copy first
						if (bUseCopy)
						{
							value = valueBuffers[id].second;
						}
						SetDataValue(data, instance, std::move(valueBuffers[id].second));
					}
				}
				valueBuffers[id].first = false;
			}

			if (ImGui::IsItemActivated())
			{
				// Update cache when this item is activated
				valueBuffers[id].second = value;

				ImGuiContext* context = ImGui::GetCurrentContext();
				// Input box is activated by double clicking, CTRL-clicking or being tabbed in
				if (ImGui::IsMouseDoubleClicked(0) ||
					(context->IO.KeyCtrl && ImGui::IsMouseClicked(0)) ||
					context->NavJustTabbedId == context->ActiveId)
				{
					// Keep writing to cache as long as input box is active
					valueBuffers[id].first = true;
				}
			}

			// Value changed during dragging
			if (bResult && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
			{
				ZE_TRACE("Value changed!");
			}

			// Value changed after dragging or inputting
			if (bIsValueChangedAfterEdit)
			{
				ZE_TRACE("Value changed after edit!");
			}
		}
		void ProcessEnumData(entt::meta_data data, entt::meta_any& instance, bool bIsSeqContainer);
		void ProcessStringData(entt::meta_data data, entt::meta_any& instance, bool bIsSeqContainer);
		void ProcessColorData(entt::meta_data data, entt::meta_any& instance, bool bIsSeqContainer);
		void ProcessTexture2DData(entt::meta_data data, entt::meta_any& instance, bool bIsSeqContainer);

	private:
		DataInspectorPanel* m_Context;

		/** Map from category to list of data of the same type in order */
		using CategorizedDatas = std::map<std::string, std::list<entt::meta_data>>;
		/** Map from type id to all its data, used to draw registered datas in DataInspectorPanel */
		std::unordered_map<uint32_t, CategorizedDatas> m_PreprocessedDatas;
		bool m_bIsPreprocessedDatasDirty{ true };
	};

}

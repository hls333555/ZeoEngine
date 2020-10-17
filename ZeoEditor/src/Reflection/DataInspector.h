#pragma once

#include <map>

#include "Engine/Core/Log.h"
#include "Engine/GameFramework/Entity.h"
#include "Reflection/ReflectionHelper.h"
#include "Engine/ImGui/MyImGui.h"

namespace ZeoEngine {

	class DataInspectorPanel;

	class DataInspector
	{
	public:
		DataInspector(DataInspectorPanel* context);

		void ProcessType(entt::meta_type type, Entity entity);

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

		/** EntityID + dataID */
		uint32_t GetUniqueDataID(entt::meta_data data);

		void PreprocessData(entt::meta_type type, entt::meta_data data);

		// NOTE: Do not pass entt::meta_handle around as it does not support copy
		bool ShouldHideData(entt::meta_data data, entt::meta_any instance);

		void EvaluateData(entt::meta_data data, entt::meta_any instance);
		void ProcessIntegralData(entt::meta_data data, entt::meta_any instance);
		void ProcessFloatingPointData(entt::meta_data data, entt::meta_any instance);
		void ProcessEnumData(entt::meta_data data, entt::meta_any instance);
		void ProcessOtherData(entt::meta_data data, entt::meta_any instance);

		void ProcessBoolData(entt::meta_data data, entt::meta_any instance);
		void ProcessStringData(entt::meta_data data, entt::meta_any instance);
		template<typename T, int32_t N = 1, typename CT = T>
		void ProcessScalarNData(entt::meta_data data, entt::meta_any instance, ImGuiDataType scalarType, CT defaultMin, CT defaultMax, const char* format = "%d")
		{
			static_assert(N == 1 || N == 2 || N == 3, "N can only be 1, 2 or 3!");

			// Map from id to value cache plus a bool flag indicating if displayed value is retrieved from cache
			static std::unordered_map<uint32_t, std::pair<bool, T>> valueBuffers;
			auto& valueRef = GetDataValueByRef<T>(data, instance);
			auto dataName = GetPropValue<const char*>(PropertyType::Name, data);
			auto speed = GetPropValue<float>(PropertyType::DragSensitivity, data);
			auto min = GetPropValue<CT>(PropertyType::ClampMin, data);
			auto minValue = min.value_or(defaultMin);
			auto max = GetPropValue<CT>(PropertyType::ClampMax, data);
			auto maxValue = max.value_or(defaultMax);
			auto id = GetUniqueDataID(data);
			// We assume getters return copy of data
			T valueCopy;
			bool bUseCopy = DoesPropExist(PropertyType::SetterAndGetter, data);
			if (bUseCopy)
			{
				// Copy the value
				valueCopy = GetDataValueByRef<T>(data, instance);
			}

			void* valuePtr = nullptr;
			void* cachedValuePtr = nullptr;
			if constexpr (N == 1) // C++17 constexpr if
			{
				valuePtr = bUseCopy ? &valueCopy : &valueRef;
				cachedValuePtr = &valueBuffers[id].second;
			}
			else
			{
				valuePtr = bUseCopy ? glm::value_ptr(valueCopy) : glm::value_ptr(valueRef);
				cachedValuePtr = glm::value_ptr(valueBuffers[id].second);
			}
			// For dragging, the value is applied immediately
			// For editing, the value is applied after completion
			bool bResult = ImGui::DragScalarN(*dataName, scalarType, valueBuffers[id].first ? cachedValuePtr : valuePtr, N, speed.value_or(1.0f), &minValue, &maxValue, format, ImGuiSliderFlags_AlwaysClamp);
			if (bUseCopy && !valueBuffers[id].first)
			{
				SetDataValue(data, instance, valueCopy);
			}

			// For multi-component drag UI, tabbing will automatically switch between those components, so we must handle deactivation first after tabbing to the next component
			bool bIsValueChangedAfterEdit = false;
			if (ImGui::IsItemDeactivatedAfterEdit())
			{
				bIsValueChangedAfterEdit = valueBuffers[id].second != (bUseCopy ? valueCopy : valueRef);
				if (valueBuffers[id].first)
				{
					// Apply cache when input box is inactive
					// Dragging will not go here
					SetDataValue(data, instance, valueBuffers[id].second);
				}
				valueBuffers[id].first = false;
			}

			if (ImGui::IsItemActivated())
			{
				// Update cache when this item is activated
				valueBuffers[id].second = bUseCopy ? valueCopy : valueRef;

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
		void ProcessColorData(entt::meta_data data, entt::meta_any instance);
		void ProcessTexture2DData(entt::meta_data data, entt::meta_any instance);

	private:
		DataInspectorPanel* m_Context;

		/** Map from category to list of data of the same type */
		using ClassifiedDatas = std::map<std::string, std::list<entt::meta_data>>;
		/** Map from type to all its data */
		std::unordered_map<entt::meta_type, ClassifiedDatas, EnttTypeHashFn> m_PreprocessedDatas;
		bool m_bIsPreprocessedDatasDirty{ true };
	};

}

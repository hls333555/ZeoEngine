#pragma once

#include "Panels/EditorPanel.h"

#include "Engine/GameFramework/Entity.h"
#include "Reflection/ReflectionHelper.h"

namespace ZeoEngine {

	class EntityInspectorPanel : public ScenePanel
	{
	public:
		using ScenePanel::ScenePanel;

	private:
		virtual void RenderPanel() override;

		void DrawInherentComponents(Entity entity);
		void DrawComponents(Entity entity);

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

		void ProcessType(entt::meta_type type, Entity entity);
		// NOTE: Do not pass entt::meta_handle around as it does not support copy
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
			// We use a non-runtime id here so that same variables on different instances share the same id
			// That's to say, when a new entity is selected, the cache will be overwritten instead of being added a new one, which in turn can save cache space
			static std::unordered_map<uint32_t, std::pair<bool, T>> valueBuffers;
			if (m_bIsSelectedEntityChanged)
			{
				for (auto& [key, value] : valueBuffers)
				{
					// When a new entity is selected, reset the flag to retrieve value from instance instead of cache 
					value.first = false;
				}
			}
			auto& valueRef = GetDataValueByRef<T>(data, instance);
			auto name = GetPropValue<const char*>(PropertyType::Name, data);
			auto speed = GetPropValue<float>(PropertyType::DragSensitivity, data);
			auto min = GetPropValue<CT>(PropertyType::ClampMin, data);
			auto minValue = min.value_or(defaultMin);
			auto max = GetPropValue<CT>(PropertyType::ClampMax, data);
			auto maxValue = max.value_or(defaultMax);

			// TODO: Consider unique runtime id?
			auto dataID = data.id();
			ImGui::PushID(dataID);
			void* valuePtr = nullptr;
			void* cachedValuePtr = nullptr;
			if constexpr (N == 1) // C++17 constexpr if
			{
				valuePtr = &valueRef;
				cachedValuePtr = &valueBuffers[dataID].second;
			}
			else
			{
				valuePtr = glm::value_ptr(valueRef);
				cachedValuePtr = glm::value_ptr(valueBuffers[dataID].second);
			}
			// For dragging, the value is applied immediately
			// For editing, the value is applied after completion
			bool bResult = ImGui::DragScalarN(*name, scalarType, valueBuffers[dataID].first ? cachedValuePtr : valuePtr, N, speed.value_or(1.0f), &minValue, &maxValue, format);

			// For multi-component drag UI, tabbing will automatically switch between those components, so we must handle deactivation first after tabbing to the next component
			bool bIsValueChangedAfterEdit = false;
			if (ImGui::IsItemDeactivatedAfterEdit())
			{
				bIsValueChangedAfterEdit = valueBuffers[dataID].second != valueRef;
				if (valueBuffers[dataID].first)
				{
					// Apply cache when input box is inactive
					// Dragging will not go here
					if constexpr (N == 1)
					{
						valueRef = std::clamp(valueBuffers[dataID].second, minValue, maxValue);
					}
					else
					{
						for (int32_t i = 0; i < N; ++i)
						{
							valueRef[i] = std::clamp(valueBuffers[dataID].second[i], minValue, maxValue);
						}
					}
				}
				valueBuffers[dataID].first = false;
			}

			if (ImGui::IsItemActivated())
			{
				// Update cache when this item is activated
				valueBuffers[dataID].second = valueRef;

				ImGuiContext* context = ImGui::GetCurrentContext();
				// Input box is activated by double clicking, CTRL-clicking or being tabbed in
				if (ImGui::IsMouseDoubleClicked(0) ||
					(context->IO.KeyCtrl && ImGui::IsMouseClicked(0)) ||
					context->NavJustTabbedId == context->ActiveId)
				{
					// Keep writing to cache as long as input box is active
					valueBuffers[dataID].first = true;
				}
			}
			ImGui::PopID();

			// Value changed during dragging
			if (bResult && !valueBuffers[dataID].first)
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

	private:
		Entity m_LastSelectedEntity;
		bool m_bIsSelectedEntityChanged;
	};

}

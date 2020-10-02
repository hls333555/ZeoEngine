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
		template<typename T>
		void ProcessScalarData(entt::meta_data data, entt::meta_any instance, ImGuiDataType scalarType, T defaultMin, T defaultMax, const char* format = "%d")
		{
			// Map from id to value cache plus a bool flag indicating if displayed value is retrieved from cache
			static std::unordered_map<uint32_t, std::pair<bool, T>> valueBuffers;
			auto& valueRef = GetDataValueByRef<T>(data, instance);
			auto name = GetPropValue<const char*>(PropertyType::Name, data);
			auto speed = GetPropValue<float>(PropertyType::DragSensitivity, data);
			auto min = GetPropValue<T>(PropertyType::ClampMin, data);
			auto minValue = min.value_or(defaultMin);
			auto max = GetPropValue<T>(PropertyType::ClampMax, data);
			auto maxValue = max.value_or(defaultMax);
			valueRef = std::clamp(valueRef, minValue, maxValue);

			auto dataID = data.id();
			ImGui::PushID(dataID);
			// For dragging, the value is applied immediately
			// For editing, the value is applied after completion
			bool bResult = ImGui::DragScalar(*name, scalarType, valueBuffers[dataID].first ? &valueBuffers[dataID].second : &valueRef, speed.value_or(1.0f), &minValue, &maxValue, format);

			if (ImGui::IsItemActivated())
			{
				//  Update cache when this item is activated
				valueBuffers[dataID].second = valueRef;

				ImGuiContext* context = ImGui::GetCurrentContext();
				// Input box is activated by double clicking, CTRL-clicking or being tabbed in
				if (ImGui::IsMouseDoubleClicked(0) ||
					(context->IO.KeyCtrl && ImGui::IsMouseClicked(0)) ||
					context->NavJustTabbedId == ImGui::GetCurrentWindow()->GetID(*name)) // GetID() must be called before PopID()!
				{
					// Keep writing to cache as long as input box is active
					valueBuffers[dataID].first = true;
				}
			}
			ImGui::PopID();

			bool bIsValueChangedAfterEdit = false;
			if (ImGui::IsItemDeactivatedAfterEdit())
			{
				bIsValueChangedAfterEdit = valueBuffers[dataID].second != valueRef;
				if (valueBuffers[dataID].first)
				{
					// Apply cache when input box is inactive
					// Dragging will not go here
					valueRef = std::clamp(valueBuffers[dataID].second, minValue, maxValue);
				}
				valueBuffers[dataID].first = false;
			}

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

	};

}

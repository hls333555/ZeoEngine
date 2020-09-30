#include "Panels/EntityInspectorPanel.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>
#include <glm/gtc/type_ptr.hpp>

#include "Engine/GameFramework/Components.h"
#include "Dockspaces/MainDockspace.h"
#include "Engine/Core/KeyCodes.h"

namespace ZeoEngine {

	void EntityInspectorPanel::RenderPanel()
	{
		Entity selectedEntity = GetContext<MainDockspace>()->m_SelectedEntity;
		if (selectedEntity)
		{
			DrawInherentComponents(selectedEntity);
			DrawComponents(selectedEntity);
		}
	}

	void EntityInspectorPanel::DrawInherentComponents(Entity entity)
	{
		if (entity.HasComponent<TagComponent>())
		{
			ProcessType(entt::resolve<TagComponent>(), entity);
		}

		if (entity.HasComponent<TransformComponent>())
		{
			ProcessType(entt::resolve<TransformComponent>(), entity);

			//if (ImGui::TreeNodeEx((void*)typeid(TransformComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Transform"))
			//{
			//	auto& transform = entity.GetComponent<TransformComponent>().Transform;

			//	ImGui::DragFloat3("Position", glm::value_ptr(transform[3]), 0.1f);

			//	ImGui::TreePop();
			//}
		}
	}

	void EntityInspectorPanel::DrawComponents(Entity entity)
	{
		GetScene()->m_Registry.visit(entity, [this, entity](const auto componentId)
		{
			const auto type = entt::resolve_type(componentId);
			if (IsTypeEqual<TagComponent>(type) || IsTypeEqual<TransformComponent>(type)) return;

			ProcessType(type, entity);
		});

		//if (entity.HasComponent<CameraComponent>())
		//{
		//	if (ImGui::TreeNodeEx((void*)typeid(CameraComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Camera"))
		//	{
		//		auto& cameraComp = entity.GetComponent<CameraComponent>();
		//		auto& camera = cameraComp.Camera;

		//		ImGui::Checkbox("Primary", &cameraComp.bIsPrimary);

		//		const char* projectionTypeStrings[] = { "Perspective", "Orthographic" };
		//		const char* currentProjectionTypeString = projectionTypeStrings[static_cast<int32_t>(camera.GetProjectionType())];
		//		if (ImGui::BeginCombo("Projection", currentProjectionTypeString))
		//		{
		//			for (int32_t i = 0; i < 2; ++i)
		//			{
		//				bool bIsSelected = currentProjectionTypeString == projectionTypeStrings[i];
		//				if (ImGui::Selectable(projectionTypeStrings[i], bIsSelected))
		//				{
		//					currentProjectionTypeString = projectionTypeStrings[i];
		//					camera.SetProjectionType(static_cast<SceneCamera::ProjectionType>(i));
		//				}
		//				if (bIsSelected)
		//				{
		//					ImGui::SetItemDefaultFocus();
		//				}
		//			}

		//			ImGui::EndCombo();
		//		}

		//		// Perspective camera settings
		//		if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
		//		{
		//			float verticalFov = glm::degrees(camera.GetPerspectiveVerticalFOV());
		//			if (ImGui::DragFloat("Vertical FOV", &verticalFov))
		//			{
		//				camera.SetPerspectiveVerticalFOV(glm::radians(verticalFov));
		//			}

		//			float perspectiveNear = camera.GetPerspectiveNearClip();
		//			if (ImGui::DragFloat("Near", &perspectiveNear))
		//			{
		//				camera.SetPerspectiveNearClip(perspectiveNear);
		//			}

		//			float perspectiveFar = camera.GetPerspectiveFarClip();
		//			if (ImGui::DragFloat("Far", &perspectiveFar))
		//			{
		//				camera.SetPerspectiveFarClip(perspectiveFar);
		//			}
		//		}

		//		// Orthographic camera settings
		//		if (camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
		//		{
		//			float orthoSize = camera.GetOrthographicSize();
		//			if (ImGui::DragFloat("Size", &orthoSize))
		//			{
		//				camera.SetOrthographicSize(orthoSize);
		//			}

		//			float orthoNear = camera.GetOrthographicNearClip();
		//			if (ImGui::DragFloat("Near", &orthoNear))
		//			{
		//				camera.SetOrthographicNearClip(orthoNear);
		//			}

		//			float orthoFar = camera.GetOrthographicFarClip();
		//			if (ImGui::DragFloat("Far", &orthoFar))
		//			{
		//				camera.SetOrthographicFarClip(orthoFar);
		//			}

		//			ImGui::Checkbox("Fixed Aspect Ratio", &cameraComp.bFixedAspectRatio);
		//		}

		//		ImGui::TreePop();
		//	}
		//}
	}

	void EntityInspectorPanel::ProcessType(entt::meta_type type, Entity entity)
	{
		const auto instance = GetTypeInstance(type, GetScene()->m_Registry, entity);
		auto name = GetPropValue<const char*>(PropertyType::Name, type);

		if (ImGui::CollapsingHeader(*name, ImGuiTreeNodeFlags_DefaultOpen))
		{
			ShowPropertyTooltip(type);
			type.data([this, instance](entt::meta_data data)
			{
				if (data.type().is_integral())
				{
					ProcessIntegralData(data, instance);
				}
				else if (data.type().is_floating_point())
				{
					ProcessFloatingPointData(data, instance);
				}
				else if (data.type().is_enum())
				{
					ProcessEnumData(data, instance);
				}
				else
				{
					ProcessOtherData(data, instance);
				}
			});
		}
	}

	void EntityInspectorPanel::ProcessIntegralData(entt::meta_data data, entt::meta_any instance)
	{
		if (IsTypeEqual<bool>(data.type()))
		{
			ProcessBoolData(data, instance);
		}
		else if (IsTypeEqual<int8_t>(data.type()))
		{
			ProcessInt8Data(data, instance);
		}
		else if (IsTypeEqual<int32_t>(data.type()))
		{
			ProcessInt32Data(data, instance);
		}
		else if (IsTypeEqual<int64_t>(data.type()))
		{
			ProcessInt64Data(data, instance);
		}
		else if (IsTypeEqual<uint8_t>(data.type()))
		{
			ProcessUInt8Data(data, instance);
		}
		else if (IsTypeEqual<uint32_t>(data.type()))
		{
			ProcessUInt32Data(data, instance);
		}
		else if (IsTypeEqual<uint64_t>(data.type()))
		{
			ProcessUInt64Data(data, instance);
		}
	}

	void EntityInspectorPanel::ProcessFloatingPointData(entt::meta_data data, entt::meta_any instance)
	{
		if (IsTypeEqual<float>(data.type()))
		{
			ProcessFloatData(data, instance);
		}
		else if (IsTypeEqual<double>(data.type()))
		{
			ProcessDoubleData(data, instance);
		}
	}

	void EntityInspectorPanel::ProcessEnumData(entt::meta_data data, entt::meta_any instance)
	{
		
	}

	void EntityInspectorPanel::ProcessOtherData(entt::meta_data data, entt::meta_any instance)
	{
		if (IsTypeEqual<std::string>(data.type()))
		{
			ProcessStringData(data, instance);
		}
		else if (IsTypeEqual<glm::vec2>(data.type()))
		{
			
		}
		else if (IsTypeEqual<glm::vec3>(data.type()))
		{
			
		}
		else if (IsTypeEqual<glm::vec4>(data.type()))
		{
			
		}
	}

	void EntityInspectorPanel::ProcessBoolData(entt::meta_data data, entt::meta_any instance)
	{
		auto& boolRef = GetDataValueByRef<bool>(data, instance);
		auto name = GetPropValue<const char*>(PropertyType::Name, data);

		auto dataID = data.id();
		ImGui::PushID(dataID);
		if (ImGui::Checkbox(*name, &boolRef))
		{
			
		}
		ImGui::PopID();
	}

	void EntityInspectorPanel::ProcessInt8Data(entt::meta_data data, entt::meta_any instance)
	{
		// Map from id to value cache plus a bool flag indicating if value is retrieved from cache
		static std::unordered_map<uint32_t, std::pair<bool, int8_t>> valueBuffers;
		auto& int8Ref = GetDataValueByRef<int8_t>(data, instance);
		auto name = GetPropValue<const char*>(PropertyType::Name, data);
		auto speed = GetPropValue<float>(PropertyType::DragSensitivity, data);
		auto min = GetPropValue<int8_t>(PropertyType::ClampMin, data);
		auto minValue = min.value_or(INT8_MIN);
		auto max = GetPropValue<int8_t>(PropertyType::ClampMax, data);
		auto maxValue = max.value_or(INT8_MAX);
		int8Ref = std::clamp(int8Ref, minValue, maxValue);

		auto dataID = data.id();
		ImGui::PushID(dataID);
		bool bResult = ImGui::DragInt_8(*name, valueBuffers[dataID].first ? &valueBuffers[dataID].second : &int8Ref, speed.value_or(1.0f), minValue, maxValue);
		
		// TODO: PushID() useful?
		if (ImGui::GetCurrentContext()->NavJustTabbedId == ImGui::GetCurrentWindow()->GetID(*name))
		{
			valueBuffers[dataID].first = true;
		}
		if (ImGui::IsItemActivated())
		{
			valueBuffers[dataID].second = int8Ref;
			// Input box is activated by tabbing, double clicking or CTRL-clicking
			if (ImGui::IsMouseDoubleClicked(0) || (ImGui::GetCurrentContext()->IO.KeyCtrl && ImGui::IsMouseClicked(0)))
			{
				valueBuffers[dataID].first = true;
			}
		}
		bool bIsValueChangedAfterEdit = false;
		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			bIsValueChangedAfterEdit = valueBuffers[dataID].second != int8Ref;
			if (valueBuffers[dataID].first)
			{
				int8Ref = std::clamp(valueBuffers[dataID].second, minValue, maxValue);
			}
			valueBuffers[dataID].first = false;
		}

		if (bResult && !valueBuffers[dataID].first)
		{
			ZE_TRACE("Value changed!");
		}

		if (bIsValueChangedAfterEdit)
		{
			ZE_TRACE("Value changed after edit!");
		}
		ImGui::PopID();
	}

	void EntityInspectorPanel::ProcessInt32Data(entt::meta_data data, entt::meta_any instance)
	{
		auto& int32Ref = GetDataValueByRef<int32_t>(data, instance);
		auto name = GetPropValue<const char*>(PropertyType::Name, data);
		auto speed = GetPropValue<float>(PropertyType::DragSensitivity, data);
		auto min = GetPropValue<int32_t>(PropertyType::ClampMin, data);
		auto minValue = min.value_or(INT32_MIN);
		auto max = GetPropValue<int32_t>(PropertyType::ClampMax, data);
		auto maxValue = max.value_or(INT32_MAX);
		int32Ref = std::clamp(int32Ref, minValue, maxValue);

		auto dataID = data.id();
		ImGui::PushID(dataID);
		if (ImGui::DragInt_32(*name, &int32Ref, speed.value_or(1.0f), minValue, maxValue))
		{
			
		}
		ImGui::PopID();
	}

	void EntityInspectorPanel::ProcessInt64Data(entt::meta_data data, entt::meta_any instance)
	{
		auto& int64Ref = GetDataValueByRef<int64_t>(data, instance);
		auto name = GetPropValue<const char*>(PropertyType::Name, data);
		auto speed = GetPropValue<float>(PropertyType::DragSensitivity, data);
		auto min = GetPropValue<int64_t>(PropertyType::ClampMin, data);
		auto minValue = min.value_or(INT64_MIN);
		auto max = GetPropValue<int64_t>(PropertyType::ClampMax, data);
		auto maxValue = max.value_or(INT64_MAX);
		int64Ref = std::clamp(int64Ref, minValue, maxValue);

		auto dataID = data.id();
		ImGui::PushID(dataID);
		if (ImGui::DragInt_64(*name, &int64Ref, speed.value_or(1.0f), minValue, maxValue))
		{
			
		}
		ImGui::PopID();
	}

	void EntityInspectorPanel::ProcessUInt8Data(entt::meta_data data, entt::meta_any instance)
	{
		auto& uint8Ref = GetDataValueByRef<uint8_t>(data, instance);
		auto name = GetPropValue<const char*>(PropertyType::Name, data);
		auto speed = GetPropValue<float>(PropertyType::DragSensitivity, data);
		auto min = GetPropValue<uint8_t>(PropertyType::ClampMin, data);
		auto minValue = min.value_or(0ui8);
		auto max = GetPropValue<uint8_t>(PropertyType::ClampMax, data);
		auto maxValue = max.value_or(UINT8_MAX);
		uint8Ref = std::clamp(uint8Ref, minValue, maxValue);

		auto dataID = data.id();
		ImGui::PushID(dataID);
		if (ImGui::DragUInt_8(*name, &uint8Ref, speed.value_or(1.0f), minValue, maxValue))
		{
			
		}
		ImGui::PopID();
	}

	void EntityInspectorPanel::ProcessUInt32Data(entt::meta_data data, entt::meta_any instance)
	{
		auto& uint32Ref = GetDataValueByRef<uint32_t>(data, instance);
		auto name = GetPropValue<const char*>(PropertyType::Name, data);
		auto speed = GetPropValue<float>(PropertyType::DragSensitivity, data);
		auto min = GetPropValue<uint32_t>(PropertyType::ClampMin, data);
		auto minValue = min.value_or(0ui32);
		auto max = GetPropValue<uint32_t>(PropertyType::ClampMax, data);
		auto maxValue = max.value_or(UINT32_MAX);
		uint32Ref = std::clamp(uint32Ref, minValue, maxValue);

		auto dataID = data.id();
		ImGui::PushID(dataID);
		if (ImGui::DragUInt_32(*name, &uint32Ref, speed.value_or(1.0f), minValue, maxValue))
		{
			
		}
		ImGui::PopID();
	}

	void EntityInspectorPanel::ProcessUInt64Data(entt::meta_data data, entt::meta_any instance)
	{
		auto& uint64Ref = GetDataValueByRef<uint64_t>(data, instance);
		auto name = GetPropValue<const char*>(PropertyType::Name, data);
		auto speed = GetPropValue<float>(PropertyType::DragSensitivity, data);
		auto min = GetPropValue<uint64_t>(PropertyType::ClampMin, data);
		auto minValue = min.value_or(0ui64);
		auto max = GetPropValue<uint64_t>(PropertyType::ClampMax, data);
		auto maxValue = max.value_or(UINT64_MAX);
		uint64Ref = std::clamp(uint64Ref, minValue, maxValue);

		auto dataID = data.id();
		ImGui::PushID(dataID);
		if (ImGui::DragUInt_64(*name, &uint64Ref, speed.value_or(1.0f), minValue, maxValue))
		{
			
		}
		ImGui::PopID();
	}

	void EntityInspectorPanel::ProcessFloatData(entt::meta_data data, entt::meta_any instance)
	{
		auto& floatRef = GetDataValueByRef<float>(data, instance);
		auto name = GetPropValue<const char*>(PropertyType::Name, data);
		auto speed = GetPropValue<float>(PropertyType::DragSensitivity, data);
		auto min = GetPropValue<float>(PropertyType::ClampMin, data);
		auto minValue = min.value_or(-FLT_MAX);
		auto max = GetPropValue<float>(PropertyType::ClampMax, data);
		auto maxValue = max.value_or(FLT_MAX);
		floatRef = std::clamp(floatRef, minValue, maxValue);

		auto dataID = data.id();
		ImGui::PushID(dataID);
		if (ImGui::DragFloat(*name, &floatRef, speed.value_or(1.0f) , minValue, maxValue, "%.2f"))
		{
			
		}
		ImGui::PopID();
	}


	void EntityInspectorPanel::ProcessDoubleData(entt::meta_data data, entt::meta_any instance)
	{
		auto& doubleRef = GetDataValueByRef<double>(data, instance);
		auto name = GetPropValue<const char*>(PropertyType::Name, data);
		auto speed = GetPropValue<float>(PropertyType::DragSensitivity, data);
		auto min = GetPropValue<double>(PropertyType::ClampMin, data);
		auto minValue = min.value_or(-DBL_MAX);
		auto max = GetPropValue<double>(PropertyType::ClampMax, data);
		auto maxValue = max.value_or(DBL_MAX);
		doubleRef = std::clamp(doubleRef, minValue, maxValue);

		auto dataID = data.id();
		ImGui::PushID(dataID);
		if (ImGui::DragDouble(*name, &doubleRef, speed.value_or(1.0f), minValue, maxValue))
		{
			
		}
		ImGui::PopID();
	}

	void EntityInspectorPanel::ProcessStringData(entt::meta_data data, entt::meta_any instance)
	{
		// Map from id to string cache plus a bool flag indicating if we are editing the text
		static std::unordered_map<uint32_t, std::pair<bool, std::string>> stringBuffers;
		auto& stringRef = GetDataValueByRef<std::string>(data, instance);
		auto name = GetPropValue<const char*>(PropertyType::Name, data);

		auto dataID = data.id();
		ImGui::PushID(dataID);
		ImGui::InputText(*name, stringBuffers[dataID].first ? &stringBuffers[dataID].second : &stringRef, ImGuiInputTextFlags_AutoSelectAll);
		ImGui::PopID();

		// Write changes to cache first
		if (ImGui::IsItemActivated())
		{
			stringBuffers[dataID].first = true;
			stringBuffers[dataID].second = stringRef;
		}
		bool bIsValueChanged = false;
		// Apply cache when user finishes editing
		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			bIsValueChanged = stringBuffers[dataID].second != stringRef;
			stringBuffers[dataID].first = false;
			stringRef = std::move(stringBuffers[dataID].second);
		}

		if (bIsValueChanged)
		{
			
		}
	}

}

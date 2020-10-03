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
			m_bIsSelectedEntityChanged = selectedEntity != m_LastSelectedEntity;
			// TODO: This can be removed if we solve the iteration order issue
			DrawInherentComponents(selectedEntity);
			DrawComponents(selectedEntity);
		}
		m_LastSelectedEntity = selectedEntity;
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
			ProcessScalarNData<int8_t>(data, instance, ImGuiDataType_S8, static_cast<int8_t>(INT8_MIN), static_cast<int8_t>(INT8_MAX));
		}
		else if (IsTypeEqual<int32_t>(data.type()))
		{
			ProcessScalarNData<int32_t>(data, instance, ImGuiDataType_S32, INT32_MIN, INT32_MAX);
		}
		else if (IsTypeEqual<int64_t>(data.type()))
		{
			ProcessScalarNData<int64_t>(data, instance, ImGuiDataType_S64, INT64_MIN, INT64_MAX);
		}
		else if (IsTypeEqual<uint8_t>(data.type()))
		{
			ProcessScalarNData<uint8_t>(data, instance, ImGuiDataType_U8, 0ui8, UINT8_MAX);
		}
		else if (IsTypeEqual<uint32_t>(data.type()))
		{
			ProcessScalarNData<uint32_t>(data, instance, ImGuiDataType_U32, 0ui32, UINT32_MAX);
		}
		else if (IsTypeEqual<uint64_t>(data.type()))
		{
			ProcessScalarNData<uint64_t>(data, instance, ImGuiDataType_U64, 0ui64, UINT64_MAX);
		}
	}

	void EntityInspectorPanel::ProcessFloatingPointData(entt::meta_data data, entt::meta_any instance)
	{
		if (IsTypeEqual<float>(data.type()))
		{
			ProcessScalarNData<float>(data, instance, ImGuiDataType_Float, -FLT_MAX, FLT_MAX, "%.2f");
		}
		else if (IsTypeEqual<double>(data.type()))
		{
			ProcessScalarNData<double>(data, instance, ImGuiDataType_Double, -DBL_MAX, DBL_MAX, "%.3f");
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
			ProcessScalarNData<glm::vec2, 2, float>(data, instance, ImGuiDataType_Float, -FLT_MAX, FLT_MAX, "%.2f");
		}
		else if (IsTypeEqual<glm::vec3>(data.type()))
		{
			ProcessScalarNData<glm::vec3, 3, float>(data, instance, ImGuiDataType_Float, -FLT_MAX, FLT_MAX, "%.3f");
		}
		else if (IsTypeEqual<glm::vec4>(data.type()))
		{
			ProcessColorData(data, instance);
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

	void EntityInspectorPanel::ProcessColorData(entt::meta_data data, entt::meta_any instance)
	{
		// Map from id to value cache plus a bool flag indicating if displayed value is retrieved from cache
		static std::unordered_map<uint32_t, std::pair<bool, glm::vec4>> valueBuffers;
		if (m_bIsSelectedEntityChanged)
		{
			for (auto& [key, value] : valueBuffers)
			{
				// When a new entity is selected, reset the flag to retrieve value from instance instead of cache 
				value.first = false;
			}
		}
		auto& vec4Ref = GetDataValueByRef<glm::vec4>(data, instance);
		auto name = GetPropValue<const char*>(PropertyType::Name, data);

		auto dataID = data.id();
		ImGui::PushID(dataID);
		bool bResult = ImGui::ColorEdit4(*name, valueBuffers[dataID].first ? glm::value_ptr(valueBuffers[dataID].second) : glm::value_ptr(vec4Ref));

		bool bIsValueChangedAfterEdit = false;
		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			bIsValueChangedAfterEdit = valueBuffers[dataID].second != vec4Ref;
			if (valueBuffers[dataID].first)
			{
				// Apply cache when input box is inactive
				// Dragging will not go here
				ImGuiContext* context = ImGui::GetCurrentContext();
				for (int32_t i = 0; i < 4; ++i)
				{
					if (context->ColorEditOptions & ImGuiColorEditFlags_Float)
					{
						vec4Ref[i] = std::clamp(valueBuffers[dataID].second[i], 0.0f, 1.0f);
					}
					else
					{
						// Internally, those values are always stored in float[0.0f, 1.0f], so we must convert them back and forth for int[0, 255]
						vec4Ref[i] = std::clamp(IM_F32_TO_INT8_UNBOUND(valueBuffers[dataID].second[i]), 0, 255) / 255.0f;
					}
				}
			}
			valueBuffers[dataID].first = false;
		}

		if (ImGui::IsItemActivated())
		{
			//  Update cache when this item is activated
			valueBuffers[dataID].second = vec4Ref;

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

}

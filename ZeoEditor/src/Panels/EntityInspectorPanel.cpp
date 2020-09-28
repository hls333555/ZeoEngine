#include "Panels/EntityInspectorPanel.h"

#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

#include "Engine/GameFramework/Components.h"
#include "Dockspaces/MainDockspace.h"
#include "Reflection/RegistrationHelper.h"

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
		if (entity.HasComponent<TagComponent>() && ImGui::CollapsingHeader(GET_PROP_NAME(entt::resolve<TagComponent>()), ImGuiTreeNodeFlags_DefaultOpen))
		{
			auto& tag = entity.GetComponent<TagComponent>().Tag;

			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strcpy_s(buffer, sizeof(buffer), tag.c_str());
			if (ImGui::InputText("Tag", buffer, sizeof(buffer)))
			{
				tag = std::string(buffer);
			}
		}

		if (entity.HasComponent<TransformComponent>() && ImGui::CollapsingHeader(GET_PROP_NAME(entt::resolve<TransformComponent>()), ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (ImGui::TreeNodeEx((void*)typeid(TransformComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Transform"))
			{
				auto& transform = entity.GetComponent<TransformComponent>().Transform;

				ImGui::DragFloat3("Position", glm::value_ptr(transform[3]), 0.1f);

				ImGui::TreePop();
			}
		}
	}

	void EntityInspectorPanel::DrawComponents(Entity entity)
	{
		GetScene()->m_Registry.visit(entity, [this](const auto componentId)
		{
			auto type = entt::resolve_type(componentId);
			if (IS_TYPE_EQUAL(type, TagComponent) || IS_TYPE_EQUAL(type, TransformComponent)) return;

			if (ImGui::CollapsingHeader(GET_PROP_NAME(type), ImGuiTreeNodeFlags_DefaultOpen))
			{
				type.data([this](entt::meta_data data)
				{
					if (data.type().is_integral())
					{
						ProcessIntegralData(data);
					}
					else if (data.type().is_enum())
					{
						ProcessEnumData(data);
					}
					else
					{
						ProcessOtherData(data);
					}
				});
			}
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

	void EntityInspectorPanel::ProcessIntegralData(entt::meta_data data)
	{
		if (IS_TYPE_EQUAL(data.type(), bool))
		{
			
		}
		else if (IS_TYPE_EQUAL(data.type(), int32_t))
		{
			
		}
		else if (IS_TYPE_EQUAL(data.type(), uint32_t))
		{
			
		}
		else if (IS_TYPE_EQUAL(data.type(), float))
		{
			
		}
		else if (IS_TYPE_EQUAL(data.type(), double))
		{
			
		}
	}

	void EntityInspectorPanel::ProcessEnumData(entt::meta_data data)
	{

	}

	void EntityInspectorPanel::ProcessOtherData(entt::meta_data data)
	{
		if (IS_TYPE_EQUAL(data.type(), std::string))
		{
			
		}
		else if (IS_TYPE_EQUAL(data.type(), glm::vec2))
		{
			
		}
		else if (IS_TYPE_EQUAL(data.type(), glm::vec3))
		{
			
		}
		else if (IS_TYPE_EQUAL(data.type(), glm::vec4))
		{
			
		}
	}

}

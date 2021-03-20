#include "ZEpch.h"
#include "Engine/Renderer/EditorCamera.h"

#include <glfw/glfw3.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <imgui.h>
#include <ImGuizmo.h>

#include "Engine/Core/Input.h"
#include "Engine/Core/KeyCodes.h"
#include "Engine/Core/MouseCodes.h"

namespace ZeoEngine {

	EditorCamera::EditorCamera(float fov, float aspectRatio, float nearClip, float farClip)
		: m_FOV(fov), m_AspectRatio(aspectRatio), m_NearClip(nearClip), m_FarClip(farClip), Camera(glm::perspective(glm::radians(fov), aspectRatio, nearClip, farClip))
	{
		UpdateView();
	}

	void EditorCamera::OnUpdate(DeltaTime dt, bool bIsViewportFocused)
	{
		m_bIsUsing = false;

		const glm::vec2 mousePos = Input::GetMousePosition();
		glm::vec2 delta = (mousePos - m_InitialMousePosition) * 0.003f;
		m_InitialMousePosition = mousePos;

		if (!bIsViewportFocused) return;

		if (ImGuizmo::IsUsing()) return;

		if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
		{
			m_bIsUsing = true;
			MousePan(delta);
		}
		
		if (Input::IsKeyPressed(Key::CameraControl))
		{
			m_bIsUsing = true;

			if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
			{
				MouseRotate(delta);
			}
			else if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
			{
				MouseZoom(delta.y);
			}
		}

		UpdateView();
	}

	void EditorCamera::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(ZE_BIND_EVENT_FUNC(EditorCamera::OnMouseScroll));
	}

	glm::vec3 EditorCamera::GetForwardVector() const
	{
		return glm::rotate(GetOrientation(), glm::vec3(0.0f, 0.0f, -1.0f));
	}

	glm::vec3 EditorCamera::GetRightVector() const
	{
		return glm::rotate(GetOrientation(), glm::vec3(1.0f, 0.0f, 0.0f));
	}

	glm::vec3 EditorCamera::GetUpVector() const
	{
		return glm::rotate(GetOrientation(), glm::vec3(0.0f, 1.0f, 0.0f));
	}

	glm::quat EditorCamera::GetOrientation() const
	{
		return glm::quat(glm::vec3(-m_Pitch, -m_Yaw, 0.0f));
	}

	void EditorCamera::UpdateProjection()
	{
		m_AspectRatio = m_ViewportWidth / m_ViewportHeight;
		m_Projection = glm::perspective(glm::radians(m_FOV), m_AspectRatio, m_NearClip, m_FarClip);
	}

	void EditorCamera::UpdateView()
	{
		// m_Yaw = m_Pitch = 0.0f; // Lock the camera's rotation
		m_Position = CalculatePosition();

		glm::quat orientation = GetOrientation();
		m_ViewMatrix = glm::translate(glm::mat4(1.0f), m_Position) * glm::toMat4(orientation);
		m_ViewMatrix = glm::inverse(m_ViewMatrix);
	}

	bool EditorCamera::OnMouseScroll(MouseScrolledEvent& e)
	{
		float delta = e.GetYOffset() * 0.1f;
		MouseZoom(delta);
		UpdateView();
		return false;
	}

	void EditorCamera::MousePan(const glm::vec2& delta)
	{
		auto [xSpeed, ySpeed] = CalculatePanSpeed();
		m_FocalPoint += -GetRightVector() * delta.x * xSpeed * m_Distance;
		m_FocalPoint += GetUpVector() * delta.y * ySpeed * m_Distance;
	}

	void EditorCamera::MouseRotate(const glm::vec2& delta)
	{
		float yawSign = GetUpVector().y < 0.0f ? -1.0f : 1.0f;
		m_Yaw += yawSign * delta.x * CalculateRotationSpeed();
		m_Pitch += delta.y * CalculateRotationSpeed();
	}

	void EditorCamera::MouseZoom(float delta)
	{
		m_Distance -= delta * CalculateZoomSpeed();
		if (m_Distance < 1.0f)
		{
			m_FocalPoint += GetForwardVector();
			m_Distance = 1.0f;
		}
	}

	glm::vec3 EditorCamera::CalculatePosition() const
	{
		return m_FocalPoint - GetForwardVector() * m_Distance;
	}

	std::pair<float, float> EditorCamera::CalculatePanSpeed() const
	{
		float x = glm::min(m_ViewportWidth / 1000.0f, 2.4f); // max = 2.4f
		float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

		float y = glm::min(m_ViewportHeight / 1000.0f, 2.4f); // max = 2.4f
		float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

		return { xFactor, yFactor };
	}

	float EditorCamera::CalculateRotationSpeed() const
	{
		return 0.8f;
	}

	float EditorCamera::CalculateZoomSpeed() const
	{
		float distance = m_Distance * 0.2f;
		distance = glm::max(distance, 0.0f);
		float speed = distance * distance;
		speed = glm::min(speed, 100.0f); // max speed = 100
		return speed;
	}

}

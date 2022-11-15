#include "ZEpch.h"
#include "Engine/Renderer/EditorCamera.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "Engine/Core/Input.h"
#include "Engine/Core/MouseCodes.h"
#include "Engine/Math/Math.h"
#include "Engine/Core/Application.h"
#include "Engine/ImGui/MyImGui.h"

namespace ZeoEngine {

	EditorCamera::EditorCamera(float fovy, float aspectRatio, float nearClip, float farClip)
		: Camera(glm::perspective(glm::radians(fovy), aspectRatio, nearClip, farClip))
		, m_FOVy(fovy), m_AspectRatio(aspectRatio), m_NearClip(nearClip), m_FarClip(farClip)
	{
		UpdateView();
	}

	void EditorCamera::OnUpdate(DeltaTime dt)
	{
		const Vec2 mousePos = Input::GetMousePosition();
		const Vec2 delta = (mousePos - m_InitialMousePosition) * 0.003f;

		m_InitialMousePosition = mousePos;

		if (m_bStartLerpToFocus)
		{
			static constexpr float focusSpeed = 13.0f;
			m_FocalPoint = Math::VInterpTo(m_FocalPoint, m_FocusTargetFocalPoint, dt, focusSpeed);
			m_Distance = Math::FInterpTo(m_Distance, m_FocusTargetDistance, dt, focusSpeed);
			if (m_FocalPoint == m_FocusTargetFocalPoint && Math::IsNearlyEqual(m_Distance, m_FocusTargetDistance))
			{
				m_bStartLerpToFocus = false;
			}
			UpdateView();
		}

		if (!m_bEnableManipulation) return;

		// WORKAROUND: Disable alt toggle behavior
		ImGui::GetCurrentContext()->NavWindowingToggleLayer = false;

		if (ImGui::IsMouseDragging(Mouse::ButtonRight) && !Input::IsKeyPressed(Key::CameraControl))
		{
			if (m_CameraMode != CameraMode::FPS)
			{
				SetFpsMode();
			}
			m_CameraMode = CameraMode::FPS;
			StartManipulating();
			MouseRotate(delta);

			if (Input::IsKeyPressed(Key::W))
			{
				m_FocalPoint += GetForwardVector() * m_FpsMoveSpeed * static_cast<float>(dt);
			}
			if (Input::IsKeyPressed(Key::S))
			{
				m_FocalPoint -= GetForwardVector() * m_FpsMoveSpeed * static_cast<float>(dt);
			}
			if (Input::IsKeyPressed(Key::D))
			{
				m_FocalPoint += GetRightVector() * m_FpsMoveSpeed * static_cast<float>(dt);
			}
			if (Input::IsKeyPressed(Key::A))
			{
				m_FocalPoint -= GetRightVector() * m_FpsMoveSpeed * static_cast<float>(dt);
			}
		}
		else if (Input::IsKeyPressed(Key::CameraControl))
		{
			if (ImGui::IsMouseDragging(Mouse::ButtonLeft))
			{
				if (m_CameraMode == CameraMode::FPS)
				{
					SetOrbitMode();
				}
				m_CameraMode = CameraMode::OrbitRotate;
				StartManipulating();
				MouseRotate(delta);
			}
			else if (ImGui::IsMouseDragging(Mouse::ButtonRight))
			{
				if (m_CameraMode == CameraMode::FPS)
				{
					SetOrbitMode();
				}
				m_CameraMode = CameraMode::Zoom;
				StartManipulating();
				MouseZoom(delta.x + delta.y);
			}
			else
			{
				StopManipulating();
			}
		}
		else if (ImGui::IsMouseDragging(Mouse::ButtonMiddle))
		{
			if (m_CameraMode == CameraMode::FPS)
			{
				SetOrbitMode();
			}
			m_CameraMode = CameraMode::Pan;
			StartManipulating();
			MousePan(delta);
		}
		else
		{
			StopManipulating();
		}

		UpdateView();
	}

	bool EditorCamera::OnMouseScroll(MouseScrolledEvent& e)
	{
		if (m_CameraMode == CameraMode::FPS)
		{
			SetOrbitMode();
		}
		const float delta = e.GetYOffset() * 0.1f;
		MouseZoom(delta);
		UpdateView();
		return false;
	}

	void EditorCamera::StartFocusEntity(Entity entity, bool bIsTeleport)
	{
		if (!entity) return;

		m_FocusTargetFocalPoint = entity.GetBounds().Origin;
		const float radius = entity.GetBounds().SphereRadius;
		m_FocusTargetDistance = radius / glm::sin(glm::radians(m_FOVy / 2.0f));
		if (bIsTeleport)
		{
			m_FocalPoint = m_FocusTargetFocalPoint;
			m_Distance = m_FocusTargetDistance;
			UpdateView();
		}
		else
		{
			m_bStartLerpToFocus = true;
		}
	}

	Vec3 EditorCamera::GetForwardVector() const
	{
		return glm::rotate(GetOrientation(), Vec3(0.0f, 0.0f, -1.0f));
	}

	Vec3 EditorCamera::GetRightVector() const
	{
		return glm::rotate(GetOrientation(), Vec3(1.0f, 0.0f, 0.0f));
	}

	Vec3 EditorCamera::GetUpVector() const
	{
		return glm::rotate(GetOrientation(), Vec3(0.0f, 1.0f, 0.0f));
	}

	Quat EditorCamera::GetOrientation() const
	{
		return Quat(Vec3(-m_Pitch, -m_Yaw, 0.0f));
	}

	void EditorCamera::UpdateProjection()
	{
		m_AspectRatio = m_ViewportWidth / m_ViewportHeight;
		m_Projection = glm::perspective(glm::radians(m_FOVy), m_AspectRatio, m_NearClip, m_FarClip);
	}

	void EditorCamera::UpdateView()
	{
		// m_Yaw = m_Pitch = 0.0f; // Lock the camera's rotation
		m_Position = CalculatePosition();

		Quat orientation = GetOrientation();
		m_ViewMatrix = glm::translate(Mat4(1.0f), m_Position) * glm::toMat4(orientation);
		m_ViewMatrix = glm::inverse(m_ViewMatrix);
	}

	void EditorCamera::MousePan(const Vec2& delta)
	{
		// Disable focusing during camera manipulation
		m_bStartLerpToFocus = false;
		auto [xSpeed, ySpeed] = CalculatePanSpeed();
		m_FocalPoint += -GetRightVector() * delta.x * xSpeed * m_Distance;
		m_FocalPoint += GetUpVector() * delta.y * ySpeed * m_Distance;
	}

	void EditorCamera::MouseRotate(const Vec2& delta)
	{
		// Disable focusing during camera manipulation
		m_bStartLerpToFocus = false;
		const float yawSign = GetUpVector().y < 0.0f ? -1.0f : 1.0f;
		m_Yaw += yawSign * delta.x * CalculateRotationSpeed();
		m_Pitch += delta.y * CalculateRotationSpeed();
	}

	void EditorCamera::MouseZoom(float delta)
	{
		// Disable focusing during camera manipulation
		m_bStartLerpToFocus = false;
		m_Distance -= delta * CalculateZoomSpeed();
		if (m_Distance < 0.5f)
		{
			m_Distance = 0.5f;
		}
	}

	Vec3 EditorCamera::CalculatePosition() const
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

	void EditorCamera::SetOrbitMode()
	{
		// Restore distance and recalculate focal point
		m_Distance = m_LastDistance;
		m_FocalPoint = m_Position + GetForwardVector() * m_Distance;
	}

	void EditorCamera::SetFpsMode()
	{
		// Backup distance
		m_LastDistance = m_Distance;
		// Set focal point to camera itself
		m_FocalPoint = m_Position;
		m_Distance = 0.0f;
	}

	void EditorCamera::StartManipulating()
	{
		m_bIsManipulating = true;
		Input::SetCursorMode(CursorMode::Locked);
		ImGui::SetMouseEnabled(false);
	}

	void EditorCamera::StopManipulating()
	{
		m_bIsManipulating = false;
		Input::SetCursorMode(CursorMode::Normal);
		ImGui::SetMouseEnabled(true);
	}

}

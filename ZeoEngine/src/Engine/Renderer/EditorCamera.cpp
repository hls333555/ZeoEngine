#include "ZEpch.h"
#include "Engine/Renderer/EditorCamera.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <imgui.h>
#include <ImGuizmo.h>

#include "Engine/Core/Input.h"
#include "Engine/Core/MouseCodes.h"
#include "Engine/Math/Math.h"
#include "Engine/Core/Application.h"
#include "Engine/Profile/BenchmarkTimer.h"

namespace ZeoEngine {

	EditorCamera::EditorCamera(float fovy, float aspectRatio, float nearClip, float farClip)
		: m_FOVy(fovy), m_AspectRatio(aspectRatio), m_NearClip(nearClip), m_FarClip(farClip), Camera(glm::perspective(glm::radians(fovy), aspectRatio, nearClip, farClip))
	{
		UpdateView();
	}

	void EditorCamera::OnUpdate(DeltaTime dt, bool bIsViewportHovered)
	{
		const glm::vec2 mousePos = Input::GetMousePosition();
		glm::vec2 delta = (mousePos - m_InitialMousePosition) * 0.003f;
		m_InitialMousePosition = mousePos;

		static const float lerpSpeed = 13.0f;
		if (m_bStartLerpToFocus)
		{
			m_FocalPoint = Math::VInterpTo(m_FocalPoint, m_FocusTargetFocalPoint, dt, lerpSpeed);
			m_Distance = Math::FInterpTo(m_Distance, m_FocusTargetDistance, dt, lerpSpeed);
			if (m_FocalPoint == m_FocusTargetFocalPoint && m_Distance == m_FocusTargetDistance)
			{
				m_bStartLerpToFocus = false;
			}
			UpdateView();
		}

		if (ImGuizmo::IsUsing()) return;

		//BEGIN_BENCHMARK()
		ProcessOrbitControl(m_bLastIsViewportHovered, delta);
		ProcessPanControl(m_bLastIsViewportHovered, delta);
		ProcessZoomControl(m_bLastIsViewportHovered, delta);
		ProcessFpsControl(m_bLastIsViewportHovered, delta, dt);
		//END_BENCHMARK()

		UpdateView();

		// Record last hovering state
		// Note that the hovering state will become inaccurate as soon as we set mouse to lock state
		m_bLastIsViewportHovered = bIsViewportHovered;
	}

	bool EditorCamera::OnMouseScroll(MouseScrolledEvent& e)
	{
		float delta = e.GetYOffset() * 0.1f;
		MouseZoom(delta);
		UpdateView();
		return false;
	}

	void EditorCamera::StartFocusEntity(Entity entity)
	{
		if (!entity) return;

		m_FocusTargetFocalPoint = entity.GetBounds().Origin;
		m_FocusTargetDistance = entity.GetBounds().SphereRadius * 1.2f / sin(glm::radians(m_FOVy / 2.0f));
		m_bStartLerpToFocus = true;
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
		m_Projection = glm::perspective(glm::radians(m_FOVy), m_AspectRatio, m_NearClip, m_FarClip);
	}

	void EditorCamera::UpdateView()
	{
		// m_Yaw = m_Pitch = 0.0f; // Lock the camera's rotation
		m_Position = CalculatePosition();

		glm::quat orientation = GetOrientation();
		m_ViewMatrix = glm::translate(glm::mat4(1.0f), m_Position) * glm::toMat4(orientation);
		m_ViewMatrix = glm::inverse(m_ViewMatrix);
	}

	void EditorCamera::ProcessOrbitControl(bool bIsViewportHovered, const glm::vec2& delta)
	{
		// Only one control mode can take effect at a time
		if (!IsControlModeReady(CameraControl_OrbitRotate)) return;

		bool& bEnableControl = m_bEnableControls[0];
		if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
		{
			if (Input::IsKeyPressed(Key::CameraControl))
			{
				// WORKAROUND: Disable alt toggle behavior
				ImGui::GetCurrentContext()->NavWindowingToggleLayer = false;

				// Set control mode once we are trying to manipulate
				EnterControlMode(CameraControl_OrbitRotate);
				// We detect first press hovering state so that the subsequent manipulation won't happen if we first press mouse in another panel
				bEnableControl = m_bIsFirstPress ? bIsViewportHovered : bEnableControl;
				m_bIsFirstPress = false;
				if (bEnableControl)
				{
					// The manipulation indeed take place, so mouse is locked
					SetMouseLock(true);
					MouseRotate(delta);
				}
			}
		}
		if (ImGui::IsMouseReleased(Mouse::ButtonLeft) && (m_CameraControlModes & CameraControl_OrbitRotate) > 0)
		{
			LeaveControlMode(CameraControl_OrbitRotate);
		}
	}

	void EditorCamera::ProcessPanControl(bool bIsViewportHovered, const glm::vec2& delta)
	{
		if (!IsControlModeReady(CameraControl_Pan)) return;

		bool& bEnableControl = m_bEnableControls[1];
		if (ImGui::IsMouseDragging(Mouse::ButtonMiddle))
		{
			EnterControlMode(CameraControl_Pan);
			bEnableControl = m_bIsFirstPress ? bIsViewportHovered : bEnableControl;
			m_bIsFirstPress = false;
			if (bEnableControl)
			{
				SetMouseLock(true);
				MousePan(delta);
			}
		}
		if (ImGui::IsMouseReleased(Mouse::ButtonMiddle) && (m_CameraControlModes & CameraControl_Pan) > 0)
		{
			LeaveControlMode(CameraControl_Pan);
		}
	}

	void EditorCamera::ProcessZoomControl(bool bIsViewportHovered, const glm::vec2& delta)
	{
		if (!IsControlModeReady(CameraControl_Zoom)) return;

		bool& bEnableControl = m_bEnableControls[2];
		if (ImGui::IsMouseDragging(Mouse::ButtonRight))
		{
			if (Input::IsKeyPressed(Key::CameraControl))
			{
				// WORKAROUND: Disable alt toggle behavior
				ImGui::GetCurrentContext()->NavWindowingToggleLayer = false;

				EnterControlMode(CameraControl_Zoom);
				bEnableControl = m_bIsFirstPress ? bIsViewportHovered : bEnableControl;
				m_bIsFirstPress = false;
				if (bEnableControl)
				{
					SetMouseLock(true);
					MouseZoom(delta.y);
				}
			}
		}
		if (ImGui::IsMouseReleased(Mouse::ButtonRight) && (m_CameraControlModes & CameraControl_Zoom) > 0)
		{
			LeaveControlMode(CameraControl_Zoom);
		}
	}

	void EditorCamera::ProcessFpsControl(bool bIsViewportHovered, const glm::vec2& delta, float dt)
	{
		if (!IsControlModeReady(CameraControl_FPS)) return;

		bool& bEnableControl = m_bEnableControls[3];
		if (Input::IsMouseButtonPressed(Mouse::ButtonRight))
		{
			// Due to IsDragging evaluation being later than IsPressed, we need to add a separate check here
			bEnableControl = m_bIsFirstPress ? bIsViewportHovered && !Input::IsKeyPressed(Key::CameraControl) : bEnableControl;
			// Clear first press only when Alt is not pressed, otherwise Alt-RightMouse control will fail
			if (!Input::IsKeyPressed(Key::CameraControl))
			{
				m_bIsFirstPress = false;
			}

			if (ImGui::IsMouseDragging(Mouse::ButtonRight))
			{
				EnterControlMode(CameraControl_FPS);
				if (bEnableControl)
				{
					SetMouseLock(true);
					if (!m_bIsRightMouseDragged)
					{
						SetFpsMode();
					}
					m_bIsRightMouseDragged = true;
					MouseRotate(delta);
				}
			}

			if (bEnableControl)
			{
				EnterControlMode(CameraControl_FPS);
				if (Input::IsKeyPressed(Key::W))
				{
					SetMouseLock(true);
					m_FocalPoint += GetForwardVector() * m_FpsMoveSpeed * dt;
				}
				if (Input::IsKeyPressed(Key::S))
				{
					SetMouseLock(true);
					m_FocalPoint -= GetForwardVector() * m_FpsMoveSpeed * dt;
				}
				if (Input::IsKeyPressed(Key::D))
				{
					SetMouseLock(true);
					m_FocalPoint += GetRightVector() * m_FpsMoveSpeed * dt;
				}
				if (Input::IsKeyPressed(Key::A))
				{
					SetMouseLock(true);
					m_FocalPoint -= GetRightVector() * m_FpsMoveSpeed * dt;
				}
			}			
		}
		if (ImGui::IsMouseReleased(Mouse::ButtonRight) && (m_CameraControlModes & CameraControl_FPS) > 0)
		{
			LeaveControlMode(CameraControl_FPS);
			if (m_bIsRightMouseDragged)
			{
				SetOrbitMode();
			}
		}
	}

	void EditorCamera::MousePan(const glm::vec2& delta)
	{
		// Disable focusing during camera manipulation
		m_bStartLerpToFocus = false;
		auto [xSpeed, ySpeed] = CalculatePanSpeed();
		m_FocalPoint += -GetRightVector() * delta.x * xSpeed * m_Distance;
		m_FocalPoint += GetUpVector() * delta.y * ySpeed * m_Distance;
	}

	void EditorCamera::MouseRotate(const glm::vec2& delta)
	{
		// Disable focusing during camera manipulation
		m_bStartLerpToFocus = false;
		float yawSign = GetUpVector().y < 0.0f ? -1.0f : 1.0f;
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

	bool EditorCamera::IsControlModeReady(uint8_t mode)
	{
		return (m_CameraControlModes & ~mode) == 0;
	}

	void EditorCamera::EnterControlMode(uint8_t mode)
	{
		m_CameraControlModes |= mode;
	}

	void EditorCamera::LeaveControlMode(uint8_t mode)
	{
		m_CameraControlModes &= ~mode;
		SetMouseLock(false);
		m_bIsFirstPress = true;
	}

	void EditorCamera::SetOrbitMode()
	{
		// Restore distance and recalculate focal point
		m_Distance = m_LastDistance;
		m_FocalPoint = m_Position + GetForwardVector() * m_Distance;
		m_bIsRightMouseDragged = false;
	}

	void EditorCamera::SetFpsMode()
	{
		// Backup distance
		m_LastDistance = m_Distance;
		// Set focal point to camera itself
		m_FocalPoint = m_Position;
		m_Distance = 0.0f;
	}

	void EditorCamera::SetMouseLock(bool bLock)
	{
		Application::Get().GetWindow().LockMouse(bLock);
	}

}

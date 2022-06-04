#pragma once

#include "Engine/Renderer/Camera.h"

#include <glm/glm.hpp>

#include "Engine/Core/DeltaTime.h"
#include "Engine/GameFramework/Entity.h"
#include "Engine/Events/MouseEvent.h"

namespace ZeoEngine {

	class EditorCamera : public Camera
	{
		friend EditorViewPanelBase;

	public:
		EditorCamera() = default;
		EditorCamera(float fovy, float aspectRatio, float nearClip, float farClip);

		void SetEnableUpdate(bool bEnable) { m_bEnableUpdate = bEnable; }

		void OnUpdate(DeltaTime dt, bool bIsViewportHovered);

		void StartFocusEntity(Entity entity);

		bool OnMouseScroll(MouseScrolledEvent& e);

		float GetDistance() const { return m_Distance; }
		void SetDistance(float distance) { m_Distance = distance; }
		float GetPitch() const { return m_Pitch; }
		float GetYaw() const { return m_Yaw; }
		float GetFpsMoveSpeed() const { return m_FpsMoveSpeed; }
		void SetFpsMoveSpeed(float speed) { m_FpsMoveSpeed = speed; }

		void SetViewportSize(float width, float height) { m_ViewportWidth = width; m_ViewportHeight = height; UpdateProjection(); }

		const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		glm::mat4 GetViewProjection() const { return m_Projection * m_ViewMatrix; }
		virtual float GetNearClip() const override { return m_NearClip; }
		virtual float GetFarClip() const override { return m_FarClip; }

		glm::vec3 GetForwardVector() const;
		glm::vec3 GetRightVector() const;
		glm::vec3 GetUpVector() const;
		const glm::vec3& GetPosition() const { return m_Position; }
		glm::quat GetOrientation() const;

	private:
		void UpdateProjection();
		void UpdateView();

		void ProcessOrbitControl(bool bIsViewportHovered, const glm::vec2& delta);
		void ProcessPanControl(bool bIsViewportHovered, const glm::vec2& delta);
		void ProcessZoomControl(bool bIsViewportHovered, const glm::vec2& delta);
		void ProcessFpsControl(bool bIsViewportHovered, const glm::vec2& delta, float dt);

		void MousePan(const glm::vec2& delta);
		void MouseRotate(const glm::vec2& delta);
		void MouseZoom(float delta);

		glm::vec3 CalculatePosition() const;

		std::pair<float, float> CalculatePanSpeed() const;
		float CalculateRotationSpeed() const;
		float CalculateZoomSpeed() const;

		bool IsControlModeReady(uint8_t mode);
		void EnterControlMode(uint8_t mode);
		void LeaveControlMode(uint8_t mode);
		void SetOrbitMode();
		void SetFpsMode();
		void SetMouseLock(bool bLock);

	private:
		bool m_bEnableUpdate = true;

		bool m_bStartLerpToFocus = false;
		glm::vec3 m_FocusTargetFocalPoint;
		float m_FocusTargetDistance;

		float m_FOVy = 50.625f, m_AspectRatio = 1.778f, m_NearClip = 0.1f, m_FarClip = 1000.0f;

		glm::mat4 m_ViewMatrix;
		glm::vec3 m_Position{ 0.0f };
		glm::vec3 m_FocalPoint{ 0.0f };

		float m_Distance = 10.0f, m_LastDistance = m_Distance;
		float m_Pitch = glm::radians(30.0f), m_Yaw = glm::radians(30.0f);
		float m_FpsMoveSpeed = 5.0f;

		float m_ViewportWidth = 1280, m_ViewportHeight = 720;

		glm::vec2 m_InitialMousePosition{ 0.0f };
		enum CameraControlMode : uint8_t
		{
			CameraControl_None = 0,
			CameraControl_OrbitRotate = ZE_BIT(0),
			CameraControl_Pan = ZE_BIT(1),
			CameraControl_FPS = ZE_BIT(2),
			CameraControl_Zoom = ZE_BIT(3),
		};
		uint8_t m_CameraControlModes = 0;
		bool m_bEnableControls[4];
		bool m_bIsFirstPress = true;
		bool m_bIsRightMouseDragged = false;
		bool m_bLastIsViewportHovered = false;
	};

}

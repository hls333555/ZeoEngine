#pragma once

#include "Engine/Renderer/Camera.h"

#include <glm/glm.hpp>

#include "Engine/Core/DeltaTime.h"
#include "Engine/GameFramework/Entity.h"
#include "Engine/Events/MouseEvent.h"

namespace ZeoEngine {

	class EditorCamera : public Camera
	{
	public:
		EditorCamera() = default;
		EditorCamera(float fovy, float aspectRatio, float nearClip, float farClip);

		void OnUpdate(DeltaTime dt);

		void SetEnableManipulation(bool bEnable) { m_bEnableManipulation = bEnable; }
		bool IsManipulating() const { return m_bIsManipulating; }

		void StartFocusEntity(Entity entity, bool bIsTeleport);

		bool OnMouseScroll(MouseScrolledEvent& e);

		float GetDistance() const { return m_Distance; }
		void SetDistance(float distance) { m_Distance = distance; }
		float GetPitch() const { return m_Pitch; }
		float GetYaw() const { return m_Yaw; }
		float GetFpsMoveSpeed() const { return m_FpsMoveSpeed; }
		void SetFpsMoveSpeed(float speed) { m_FpsMoveSpeed = speed; }

		void SetViewportSize(float width, float height) { m_ViewportWidth = width; m_ViewportHeight = height; UpdateProjection(); }

		const Mat4& GetViewMatrix() const { return m_ViewMatrix; }
		Mat4 GetViewProjection() const { return m_Projection * m_ViewMatrix; }
		virtual float GetNearClip() const override { return m_NearClip; }
		virtual float GetFarClip() const override { return m_FarClip; }
		void SetFarClip(float farClip) { m_FarClip = farClip; UpdateProjection(); }

		Vec3 GetForwardVector() const;
		Vec3 GetRightVector() const;
		Vec3 GetUpVector() const;
		const Vec3& GetPosition() const { return m_Position; }
		glm::quat GetOrientation() const;

	private:
		void UpdateProjection();
		void UpdateView();

		void MousePan(const Vec2& delta);
		void MouseRotate(const Vec2& delta);
		void MouseZoom(float delta);

		Vec3 CalculatePosition() const;

		std::pair<float, float> CalculatePanSpeed() const;
		float CalculateRotationSpeed() const;
		float CalculateZoomSpeed() const;

		void SetOrbitMode();
		void SetFpsMode();
		void StartManipulating();
		void StopManipulating();

	private:
		bool m_bEnableManipulation = true;
		bool m_bIsManipulating = false;

		bool m_bStartLerpToFocus = false;
		Vec3 m_FocusTargetFocalPoint;
		float m_FocusTargetDistance;

		float m_FOVy = 50.625f, m_AspectRatio = 1.778f, m_NearClip = 0.1f, m_FarClip = 1000.0f;

		Mat4 m_ViewMatrix;
		Vec3 m_Position{ 0.0f };
		Vec3 m_FocalPoint{ 0.0f };

		float m_Distance = 10.0f, m_LastDistance = m_Distance;
		float m_Pitch = glm::radians(30.0f), m_Yaw = glm::radians(30.0f);
		float m_FpsMoveSpeed = 5.0f;

		float m_ViewportWidth = 1280, m_ViewportHeight = 720;

		Vec2 m_InitialMousePosition{ 0.0f };
		enum class CameraMode
		{
			None,
			OrbitRotate,
			Pan,
			FPS ,
			Zoom,
		};
		CameraMode m_CameraMode;
	};

}

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

		void OnUpdate(DeltaTime dt, bool bIsViewportFocused);

		/** Returns true when being manipulated. */
		bool IsUsing() const { return m_bIsUsing; }

		void StartFocusEntity(Entity entity);

		bool OnMouseScroll(MouseScrolledEvent& e);

		float GetDistance() const { return m_Distance; }
		void SetDistance(float distance) { m_Distance = distance; }

		void SetViewportSize(float width, float height) { m_ViewportWidth = width; m_ViewportHeight = height; UpdateProjection(); }

		const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		glm::mat4 GetViewProjection() const { return m_Projection * m_ViewMatrix; }
		float GetNearClip() const { return m_NearClip; }
		float GetFarClip() const { return m_FarClip; }

		glm::vec3 GetForwardVector() const;
		glm::vec3 GetRightVector() const;
		glm::vec3 GetUpVector() const;
		const glm::vec3& GetPosition() const { return m_Position; }
		glm::quat GetOrientation() const;

		float GetPitch() const { return m_Pitch; }
		float GetYaw() const { return m_Yaw; }

	private:
		void UpdateProjection();
		void UpdateView();

		void MousePan(const glm::vec2& delta);
		void MouseRotate(const glm::vec2& delta);
		void MouseZoom(float delta);

		glm::vec3 CalculatePosition() const;

		std::pair<float, float> CalculatePanSpeed() const;
		float CalculateRotationSpeed() const;
		float CalculateZoomSpeed() const;

	private:
		bool m_bIsUsing = false;

		bool m_bStartLerpToFocus = false;
		Entity m_FocusedEntity;

		float m_FOVy = 90.0f, m_AspectRatio = 1.778f, m_NearClip = 0.1f, m_FarClip = 1000.0f;

		glm::mat4 m_ViewMatrix;
		glm::vec3 m_Position{ 0.0f };
		glm::vec3 m_FocalPoint{ 0.0f };

		glm::vec2 m_InitialMousePosition{ 0.0f };

		float m_Distance = 10.0f;
		float m_Pitch = glm::radians(30.0f), m_Yaw = glm::radians(30.0f);

		float m_ViewportWidth = 1280, m_ViewportHeight = 720;
	};

}

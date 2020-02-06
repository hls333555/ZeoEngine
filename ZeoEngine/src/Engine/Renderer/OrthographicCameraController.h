#pragma once

#include "Engine/Renderer/OrthographicCamera.h"
#include "Engine/Core/DeltaTime.h"

#include "Engine/Events/ApplicationEvent.h"
#include "Engine/Events/MouseEvent.h"
#include "Engine/Events/KeyEvent.h"

namespace ZeoEngine {

	class OrthographicCameraController
	{
	public:
		OrthographicCameraController(float aspectRatio, bool bRotation = false);

		void OnUpdate(DeltaTime dt);
		void OnEvent(Event& e);

		OrthographicCamera& GetCamera() { return m_Camera; }
		const OrthographicCamera& GetCamera() const { return m_Camera; }

		float GetZoomLevel() const { return m_ZoomLevel; }
		void SetZoomLevel(float level) { m_ZoomLevel = level; }

		/** Wrapper function for setting camera's projection. */
		void UpdateProjection(float aspectRatio);

	private:
		bool OnMouseScrolled(MouseScrolledEvent& e);
		bool OnKeyPressed(KeyPressedEvent& e);
		bool OnWindowResized(WindowResizeEvent& e);

	private:
		OrthographicCamera m_Camera;
		float m_AspectRatio;
		float m_ZoomLevel;
		float m_CameraTranslationSpeed, m_CameraRotationSpeed;
		bool m_bRotation;

		/** Whenever you change camera position via m_Camera.SetPosition(), you should update this either */
		glm::vec3 m_CameraPosition = { 0.0f, 0.0f, 0.0f };
		/** In degrees, in the anti-clockwise direction */
		float m_CameraRotation = 0.0f;

		glm::vec2 m_LastPressedMousePosition{ 0.0f, 0.0f };
		bool m_bMiddleMouseButtonFirstPressed = true;
		
	};

}

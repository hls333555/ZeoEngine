#pragma once

#include "Engine/Renderer/OrthographicCamera.h"
#include "Engine/Core/DeltaTime.h"

#include "Engine/Events/ApplicationEvent.h"
#include "Engine/Events/MouseEvent.h"

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

	private:
		bool OnMouseScrolled(MouseScrolledEvent& e);
		bool OnWindowResized(WindowResizeEvent& e);

	private:
		float m_AspectRatio;
		float m_ZoomLevel = 1.f;
		OrthographicCamera m_Camera;

		bool m_bRotation;
		glm::vec3 m_CameraPosition = { 0.f, 0.f, 0.f };
		float m_CameraRotation = 0.f;
		float m_CameraTranslationSpeed = 5.f, m_CameraRotationSpeed = 180.f;
	};

}

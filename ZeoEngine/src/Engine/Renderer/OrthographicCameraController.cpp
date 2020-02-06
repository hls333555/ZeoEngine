#include "ZEpch.h"
#include "Engine/Renderer/OrthographicCameraController.h"

#include "Engine/Core/Input.h"
#include "Engine/Core/KeyCodes.h"

namespace ZeoEngine {

#define DEFAULT_ZOOMLEVEL 3.0f

	OrthographicCameraController::OrthographicCameraController(float aspectRatio, bool bRotation)
		: m_AspectRatio(aspectRatio)
		, m_ZoomLevel(DEFAULT_ZOOMLEVEL)
		, m_Camera(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel)
		, m_CameraTranslationSpeed(5.0f)
		, m_CameraRotationSpeed(180.0f)
		, m_bRotation(bRotation)
	{
	}

	void OrthographicCameraController::OnUpdate(DeltaTime dt)
	{
		ZE_PROFILE_FUNCTION();

		// Move speed is set based on the zoom level
		m_CameraTranslationSpeed = m_ZoomLevel / 2.0f;
		
		// Pan camera view by holding middle mouse button
		// TODO: This does not go well with camera rotation
		if (Input::IsMouseButtonPressed(2))
		{
			auto [x, y] = Input::GetMousePosition();
			if (!m_bMiddleMouseButtonFirstPressed)
			{
				m_CameraPosition.x -= (x - m_LastPressedMousePosition.x) * m_CameraTranslationSpeed * dt;
				m_CameraPosition.y += (y - m_LastPressedMousePosition.y) * m_CameraTranslationSpeed * dt;
				m_Camera.SetPosition(m_CameraPosition);
			}
			m_bMiddleMouseButtonFirstPressed = false;
			m_LastPressedMousePosition = { x, y };
		}
		if (Input::IsMouseButtonReleased(2))
		{
			m_bMiddleMouseButtonFirstPressed = true;
		}

		//if (Input::IsKeyPressed(ZE_KEY_A))
		//{
		//	m_CameraPosition.x -= m_CameraTranslationSpeed * cos(glm::radians(m_CameraRotation)) * dt;
		//	m_CameraPosition.y -= m_CameraTranslationSpeed * sin(glm::radians(m_CameraRotation)) * dt;
		//}
		//if (Input::IsKeyPressed(ZE_KEY_D))
		//{
		//	m_CameraPosition.x += m_CameraTranslationSpeed * cos(glm::radians(m_CameraRotation)) * dt;
		//	m_CameraPosition.y += m_CameraTranslationSpeed * sin(glm::radians(m_CameraRotation)) * dt;
		//}
		//if (Input::IsKeyPressed(ZE_KEY_W))
		//{
		//	m_CameraPosition.x += m_CameraTranslationSpeed * -sin(glm::radians(m_CameraRotation)) * dt;
		//	m_CameraPosition.y += m_CameraTranslationSpeed * cos(glm::radians(m_CameraRotation)) * dt;
		//}
		//if (Input::IsKeyPressed(ZE_KEY_S))
		//{
		//	m_CameraPosition.x -= m_CameraTranslationSpeed * -sin(glm::radians(m_CameraRotation)) * dt;
		//	m_CameraPosition.y -= m_CameraTranslationSpeed * cos(glm::radians(m_CameraRotation)) * dt;
		//}
		//m_Camera.SetPosition(m_CameraPosition);

		//if (m_bRotation)
		//{
		//	if (Input::IsKeyPressed(ZE_KEY_Q))
		//	{
		//		m_CameraRotation -= m_CameraRotationSpeed * dt;
		//	}
		//	if (Input::IsKeyPressed(ZE_KEY_E))
		//	{
		//		m_CameraRotation += m_CameraRotationSpeed * dt;
		//	}
		//	if (m_CameraRotation > 180.0f)
		//	{
		//		m_CameraRotation -= 360.0f;
		//	}
		//	else if (m_CameraRotation < -180.0f)
		//	{
		//		m_CameraRotation += 360.0f;
		//	}
		//	m_Camera.SetRotation(m_CameraRotation);
		//}
		
	}

	void OrthographicCameraController::OnEvent(Event& e)
	{
		ZE_PROFILE_FUNCTION();

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(ZE_BIND_EVENT_FUNC(OrthographicCameraController::OnMouseScrolled));
		dispatcher.Dispatch<KeyPressedEvent>(ZE_BIND_EVENT_FUNC(OrthographicCameraController::OnKeyPressed));
		// NOTE: Updating projection on window resizing is processed in EditorLayer now
		//dispatcher.Dispatch<WindowResizeEvent>(ZE_BIND_EVENT_FUNC(OrthographicCameraController::OnWindowResized));
	}

	void OrthographicCameraController::UpdateProjection(float aspectRatio)
	{
		m_AspectRatio = aspectRatio;
		m_Camera.SetProjection(-aspectRatio * m_ZoomLevel, aspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
	}

	bool OrthographicCameraController::OnMouseScrolled(MouseScrolledEvent& e)
	{
		ZE_PROFILE_FUNCTION();

		m_ZoomLevel -= e.GetYOffset() * 0.25f;
		m_ZoomLevel = std::max(m_ZoomLevel, 0.25f);
		UpdateProjection(m_AspectRatio);
		return false;
	}

	bool OrthographicCameraController::OnKeyPressed(KeyPressedEvent& e)
	{
		ZE_PROFILE_FUNCTION();

		// Reset camera view by pressing C
		if (e.GetKeyCode() == ZE_KEY_C)
		{
			m_CameraPosition = { 0.0f, 0.0f, 0.0f };
			m_Camera.SetPosition(m_CameraPosition);
			m_CameraRotation = 0.0f;
			m_Camera.SetRotation(0.0f);
			SetZoomLevel(DEFAULT_ZOOMLEVEL);
			UpdateProjection(m_AspectRatio);
		}
		return false;
	}

	bool OrthographicCameraController::OnWindowResized(WindowResizeEvent& e)
	{
		ZE_PROFILE_FUNCTION();

		UpdateProjection((float)e.GetWidth() / (float)e.GetHeight());
		return false;
	}

}

#include "ZEpch.h"
#include "Engine/Renderer/OrthographicCameraController.h"

#include "Engine/Core/Input.h"
#include "Engine/Core/KeyCodes.h"

namespace ZeoEngine {

	OrthographicCameraController::OrthographicCameraController(float aspectRatio, bool bRotation)
		: m_AspectRatio(aspectRatio)
		, m_Camera(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel)
		, m_bRotation(bRotation)
	{
	}

	void OrthographicCameraController::OnUpdate(DeltaTime dt)
	{
		ZE_PROFILE_FUNCTION();

		// Move speed is set based on the zoom level
		m_CameraTranslationSpeed = m_ZoomLevel;

		if (Input::IsKeyPressed(ZE_KEY_A))
		{
			m_CameraPosition.x -= m_CameraTranslationSpeed * cos(glm::radians(m_CameraRotation)) * dt;
			m_CameraPosition.y -= m_CameraTranslationSpeed * sin(glm::radians(m_CameraRotation)) * dt;
		}
		if (Input::IsKeyPressed(ZE_KEY_D))
		{
			m_CameraPosition.x += m_CameraTranslationSpeed * cos(glm::radians(m_CameraRotation)) * dt;
			m_CameraPosition.y += m_CameraTranslationSpeed * sin(glm::radians(m_CameraRotation)) * dt;
		}
		if (Input::IsKeyPressed(ZE_KEY_W))
		{
			m_CameraPosition.x += m_CameraTranslationSpeed * -sin(glm::radians(m_CameraRotation)) * dt;
			m_CameraPosition.y += m_CameraTranslationSpeed * cos(glm::radians(m_CameraRotation)) * dt;
		}
		if (Input::IsKeyPressed(ZE_KEY_S))
		{
			m_CameraPosition.x -= m_CameraTranslationSpeed * -sin(glm::radians(m_CameraRotation)) * dt;
			m_CameraPosition.y -= m_CameraTranslationSpeed * cos(glm::radians(m_CameraRotation)) * dt;
		}
		m_Camera.SetPosition(m_CameraPosition);

		if (m_bRotation)
		{
			if (Input::IsKeyPressed(ZE_KEY_Q))
			{
				m_CameraRotation -= m_CameraRotationSpeed * dt;
			}
			if (Input::IsKeyPressed(ZE_KEY_E))
			{
				m_CameraRotation += m_CameraRotationSpeed * dt;
			}
			if (m_CameraRotation > 180.0f)
			{
				m_CameraRotation -= 360.0f;
			}
			else if (m_CameraRotation < -180.0f)
			{
				m_CameraRotation += 360.0f;
			}
			ZE_INFO(m_CameraRotation);
			m_Camera.SetRotation(m_CameraRotation);
		}
		
	}

	void OrthographicCameraController::OnEvent(Event& e)
	{
		ZE_PROFILE_FUNCTION();

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(ZE_BIND_EVENT_FUNC(OrthographicCameraController::OnMouseScrolled));
		dispatcher.Dispatch<WindowResizeEvent>(ZE_BIND_EVENT_FUNC(OrthographicCameraController::OnWindowResized));
	}

	bool OrthographicCameraController::OnMouseScrolled(MouseScrolledEvent& e)
	{
		ZE_PROFILE_FUNCTION();

		m_ZoomLevel -= e.GetYOffset() * 0.25f;
		m_ZoomLevel = std::max(m_ZoomLevel, 0.25f);
		m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);

		return false;
	}

	bool OrthographicCameraController::OnWindowResized(WindowResizeEvent& e)
	{
		ZE_PROFILE_FUNCTION();

		m_AspectRatio = (float)e.GetWidth() / (float)e.GetHeight();
		m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
		
		return false;
	}

}

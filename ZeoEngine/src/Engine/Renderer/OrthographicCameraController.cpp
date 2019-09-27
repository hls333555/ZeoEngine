#include "ZEpch.h"
#include "OrthographicCameraController.h"

#include "Engine/Input.h"
#include "Engine/KeyCodes.h"
#include "glad/glad.h"

namespace ZeoEngine {

	OrthographicCameraController::OrthographicCameraController(float aspectRatio, bool bRotation)
		: m_AspectRatio(aspectRatio)
		, m_Camera(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel)
		, m_bRotation(bRotation)
	{
	}

	void OrthographicCameraController::OnUpdate(DeltaTime dt)
	{
		// Move speed is set based on the zoom level
		m_CameraTranslationSpeed = m_ZoomLevel;

		if (Input::IsKeyPressed(ZE_KEY_A))
		{
			m_CameraPosition.x -= m_CameraTranslationSpeed * dt;
		}
		if (Input::IsKeyPressed(ZE_KEY_D))
		{
			m_CameraPosition.x += m_CameraTranslationSpeed * dt;
		}
		if (Input::IsKeyPressed(ZE_KEY_W))
		{
			m_CameraPosition.y += m_CameraTranslationSpeed * dt;
		}
		if (Input::IsKeyPressed(ZE_KEY_S))
		{
			m_CameraPosition.y -= m_CameraTranslationSpeed * dt;
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
			m_Camera.SetRotation(m_CameraRotation);
		}
		
	}

	void OrthographicCameraController::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(ZE_BIND_EVENT_FUNC(OrthographicCameraController::OnMouseScrolled));
		dispatcher.Dispatch<WindowResizeEvent>(ZE_BIND_EVENT_FUNC(OrthographicCameraController::OnWindowResized));
	}

	bool OrthographicCameraController::OnMouseScrolled(MouseScrolledEvent& e)
	{
		m_ZoomLevel -= e.GetYOffset() * 0.25f;
		m_ZoomLevel = std::max(m_ZoomLevel, 0.25f);
		m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);

		return false;
	}

	bool OrthographicCameraController::OnWindowResized(WindowResizeEvent& e)
	{
		// TODO:
		glViewport(0, 0, e.GetWidth(), e.GetHeight());

		m_AspectRatio = (float)e.GetWidth() / (float)e.GetHeight();
		m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
		
		return false;
	}

}

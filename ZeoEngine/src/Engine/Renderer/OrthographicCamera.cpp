#include "ZEpch.h"
#include "Engine/Renderer/OrthographicCamera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace ZeoEngine {

	OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top)
		: m_ViewMatrix(1.0f)
	{
		ZE_PROFILE_FUNCTION();

		SetProjection(left, right, bottom, top);
	}

	void OrthographicCamera::SetProjection(float left, float right, float bottom, float top)
	{
		ZE_PROFILE_FUNCTION();

		m_ProjectionMatrix = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
		m_CameraBounds = { left, right, bottom, top };
	}

	void OrthographicCamera::RecalculateViewMatrix()
	{
		ZE_PROFILE_FUNCTION();

		Mat4 transform = glm::translate(Mat4(1.0f), m_Position) *
			// Rotate along Z-axis, note that radians is used instead of degrees!
			glm::rotate(Mat4(1.0f), glm::radians(m_Rotation), Vec3(0, 0, 1));
	
		m_ViewMatrix = glm::inverse(transform);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

}

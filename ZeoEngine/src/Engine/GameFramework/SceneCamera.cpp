#include "ZEpch.h"
#include "Engine/GameFramework/SceneCamera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace ZeoEngine {

	SceneCamera::SceneCamera()
	{
		RecalculateProjection();
	}

	glm::mat4 SceneCamera::CalculatePerspectiveProjection(float nearClip, float farClip) const
	{
		ZE_CORE_ASSERT(m_ProjectionType == ProjectionType::Perspective);
		return glm::perspective(m_PerspectiveFOV, m_AspectRatio, nearClip, farClip);
	}

	void SceneCamera::SetPerspective(float verticalFOV, float nearClip, float farClip)
	{
		m_ProjectionType = ProjectionType::Perspective;
		m_PerspectiveFOV = verticalFOV;
		m_PerspectiveNear = nearClip;
		m_PerspectiveFar = farClip;
		RecalculateProjection();
	}

	void SceneCamera::SetOrthographic(float size, float nearClip, float farClip)
	{
		m_ProjectionType = ProjectionType::Orthographic;
		m_OrthographicSize = size;
		m_OrthographicNear = nearClip;
		m_OrthographicFar = farClip;
		RecalculateProjection();
	}

	void SceneCamera::SetViewportSize(const glm::vec2& size)
	{
		m_AspectRatio = size.x / size.y;
		RecalculateProjection();
	}

	void SceneCamera::RecalculateProjection()
	{
		if (m_ProjectionType == ProjectionType::Perspective)
		{
			m_Projection = CalculatePerspectiveProjection(m_PerspectiveNear, m_PerspectiveFar);
		}
		else
		{
			float orthoLeft = -m_OrthographicSize * m_AspectRatio * 0.5f;
			float orthoRight = m_OrthographicSize * m_AspectRatio * 0.5f;
			float orthoBottom = -m_OrthographicSize * 0.5f;
			float orthoTop = m_OrthographicSize * 0.5f;
			m_Projection = glm::ortho(orthoLeft, orthoRight, orthoBottom, orthoTop, m_OrthographicNear, m_OrthographicFar);
		}
	}

}

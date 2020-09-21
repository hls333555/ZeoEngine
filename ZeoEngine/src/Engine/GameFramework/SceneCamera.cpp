#include "ZEpch.h"
#include "Engine/GameFramework/SceneCamera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace ZeoEngine {

	SceneCamera::SceneCamera()
	{
		RecalculateProjection();
	}

	void SceneCamera::SetOrthographic(float size, float nearClip, float farClip)
	{
		m_OrthographicSize = size;
		m_OrthofraphicNear = nearClip;
		m_OrthofraphicFar = farClip;
		RecalculateProjection();
	}

	void SceneCamera::SetViewportSize(const glm::vec2& size)
	{
		m_AspectRatio = size.x / size.y;
		RecalculateProjection();
	}

	void SceneCamera::RecalculateProjection()
	{
		float orthoLeft = -m_OrthographicSize * m_AspectRatio * 0.5f;
		float orthoRight = m_OrthographicSize * m_AspectRatio * 0.5f;
		float orthoBottom = -m_OrthographicSize * 0.5f;
		float orthoTop = m_OrthographicSize * 0.5f;
		m_Projection = glm::ortho(orthoLeft, orthoRight, orthoBottom, orthoTop, m_OrthofraphicNear, m_OrthofraphicFar);
	}

}

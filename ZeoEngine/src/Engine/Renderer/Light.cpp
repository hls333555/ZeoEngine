#include "ZEpch.h"
#include "Engine/Renderer/Light.h"

#include <glm/gtx/quaternion.hpp>

namespace ZeoEngine {

	Vec3 Light::CalculateDirection(const Vec3& rotation) const
	{
		return glm::rotate(glm::quat(rotation), { 0.0f, 0.0f, -1.0f });
	}

}

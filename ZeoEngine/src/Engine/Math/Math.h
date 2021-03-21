#pragma once

#include <glm/glm.hpp>

#include "Engine/Core/DeltaTime.h"

namespace ZeoEngine::Math {

#define KINDA_SMALL_NUMBER	(1.e-4f)

	bool DecomposeTransform(const glm::mat4& transfrom, glm::vec3& outTranslation, glm::vec3& outRotation, glm::vec3& outScale);

	/** Interpolate vector from current to target. Scaled by distance to Target, so it has a strong start speed and ease out. */
	glm::vec3 InterpTo(const glm::vec3& current, const glm::vec3& target, DeltaTime dt, float interpSpeed);

}

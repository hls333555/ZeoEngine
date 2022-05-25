#pragma once

#include <glm/glm.hpp>

#include "Engine/Core/DeltaTime.h"

namespace ZeoEngine::Math {

#define SMALL_NUMBER	(1.e-8f)
#define KINDA_SMALL_NUMBER	(1.e-4f)

	bool DecomposeTransform(const glm::mat4& transfrom, glm::vec3& outTranslation, glm::vec3& outRotation, glm::vec3& outScale);

	glm::vec3 GetTranslationFromTransform(const glm::mat4& transfrom);

	/** Interpolate float from current to target. Scaled by distance to Target, so it has a strong start speed and ease out. */
	float FInterpTo(float current, float target, DeltaTime dt, float interpSpeed);

	/** Interpolate vector from current to target. Scaled by distance to Target, so it has a strong start speed and ease out. */
	glm::vec3 VInterpTo(const glm::vec3& current, const glm::vec3& target, DeltaTime dt, float interpSpeed);

}

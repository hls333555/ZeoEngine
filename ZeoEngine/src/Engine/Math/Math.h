#pragma once

#include <glm/glm.hpp>

namespace ZeoEngine::Math {

	bool DecomposeTransform(const glm::mat4& transfrom, glm::vec3& outTranslation, glm::vec3& outRotation, glm::vec3& outScale);

}

#include "ZEpch.h"
#include "Engine/Renderer/Light.h"

#include <glm/gtx/quaternion.hpp>

namespace ZeoEngine {

    glm::vec3 Light::CalculateDirection(const glm::vec3& rotation) const
    {
        return glm::normalize(glm::rotate(glm::quat(rotation), { 0.0f, 0.0f, -1.0f }));
    }

}

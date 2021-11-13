#include "ZEpch.h"
#include "Engine/Renderer/Light.h"

#include <glm/gtx/quaternion.hpp>

namespace ZeoEngine {

    glm::vec3 DirectionalLight::CalculateDirection(const glm::vec3& rotation) const
    {
        return glm::rotate(glm::quat(rotation), glm::vec3(0.0f, 0.0f, -1.0f));
    }

}

#include "ZEpch.h"
#include "Engine/Renderer/Light.h"

#include <glm/gtx/quaternion.hpp>

namespace ZeoEngine {

    glm::vec3 Light::CalculateDirection(const glm::vec3& rotation) const
    {
        return glm::rotate(glm::quat(rotation), { 0.0f, 0.0f, -1.0f });
    }

    DirectionalLight::DirectionalLight()
    {
        // TODO:
        m_Projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, -10.0f, 20.0f);
    }

}

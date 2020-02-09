#pragma once

#include <glm/glm.hpp>
#include <imgui.h>
#include <imgui_internal.h>

namespace ZeoEngine {

	class OrthographicCamera;

	glm::vec2 ProjectScreenToWorld2D(const glm::vec2& screenPosition, ImGuiWindow* gameViewWindow, OrthographicCamera* camera);
	glm::vec2 ProjectWorldToScreen2D(const glm::vec2& worldPosition, ImGuiWindow* gameViewWindow, OrthographicCamera* camera);

}

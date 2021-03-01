#pragma once

#include <glm/glm.hpp>

#include "Engine/Core/Core.h"
#include "Engine/Core/KeyCodes.h"
#include "Engine/Core/MouseCodes.h"

namespace ZeoEngine {

	class Input
	{
	public:
		static bool IsKeyPressed(const KeyCode keycode);
		static bool IsKeyReleased(const KeyCode keycode);
		
		static bool IsMouseButtonPressed(const MouseCode button);
		static bool IsMouseButtonReleased(const MouseCode button);
		static glm::vec2 GetMousePosition();
		static float GetMouseX();
		static float GetMouseY();
	
	};

}

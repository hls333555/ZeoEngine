#pragma once

#include <glm/glm.hpp>

#include "Engine/Core/KeyCodes.h"
#include "Engine/Core/MouseCodes.h"

namespace ZeoEngine {

	enum class CursorMode
	{
		Normal = 0,
		Hidden = 1,
		Locked = 2
	};

	class Input
	{
	public:
		static bool IsKeyPressed(const KeyCode keycode);
		static bool IsKeyReleased(const KeyCode keycode);
		
		static bool IsMouseButtonPressed(const MouseCode button);
		static bool IsMouseButtonReleased(const MouseCode button);
		static Vec2 GetMousePosition();
		static float GetMouseX();
		static float GetMouseY();
		static CursorMode GetCursorMode();
		static void SetCursorMode(CursorMode mode);

	};

}

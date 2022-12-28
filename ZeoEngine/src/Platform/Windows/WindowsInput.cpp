#include "ZEpch.h"
#include "Engine/Core/Input.h"

#include "Engine/Core/Application.h"
#include <GLFW/glfw3.h>

namespace ZeoEngine {

	bool Input::IsKeyPressed(const KeyCode keycode)
	{
		auto* window = Application::Get().GetActiveNativeWindow();
		auto state = glfwGetKey(window, keycode);
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool Input::IsKeyReleased(const KeyCode keycode)
	{
		auto* window = Application::Get().GetActiveNativeWindow();
		auto state = glfwGetKey(window, keycode);
		return state == GLFW_RELEASE;
	}

	bool Input::IsMouseButtonPressed(const MouseCode button)
	{
		auto* window = Application::Get().GetActiveNativeWindow();
		auto state = glfwGetMouseButton(window, button);
		return state == GLFW_PRESS;
	}

	bool Input::IsMouseButtonReleased(const MouseCode button)
	{
		auto* window = Application::Get().GetActiveNativeWindow();
		auto state = glfwGetMouseButton(window, button);
		return state == GLFW_RELEASE;
	}

	Vec2 Input::GetMousePosition()
	{
		auto* window = Application::Get().GetActiveNativeWindow();
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		return { static_cast<float>(xpos), static_cast<float>(ypos) };
	}

	float Input::GetMouseX()
	{
		return GetMousePosition().x;
	}

	float Input::GetMouseY()
	{
		return GetMousePosition().y;
	}

	CursorMode Input::GetCursorMode()
	{
		auto* window = Application::Get().GetActiveNativeWindow();
		return static_cast<CursorMode>((glfwGetInputMode(window, GLFW_CURSOR) - GLFW_CURSOR_NORMAL));
	}

	void Input::SetCursorMode(CursorMode mode)
	{
		auto* window = Application::Get().GetActiveNativeWindow();
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL + static_cast<int>(mode));
	}

}

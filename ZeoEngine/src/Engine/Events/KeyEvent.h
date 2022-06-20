#pragma once

#include <sstream>

#include "Engine/Events/Event.h"
#include "Engine/Core/KeyCodes.h"

namespace ZeoEngine {

	class KeyEvent : public Event
	{
	public:
		KeyCode GetKeyCode() const { return m_KeyCode; }

		EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)

	protected:
		KeyEvent(GLFWwindow* window, const KeyCode keycode)
			: Event(window), m_KeyCode(keycode) {}

		KeyCode m_KeyCode;

	};

	class KeyPressedEvent : public KeyEvent
	{
	public:
		KeyPressedEvent(GLFWwindow* window, const KeyCode keycode, const U16 repeatCount)
			: KeyEvent(window, keycode), m_RepeatCount(repeatCount) {}

		U16 GetRepeatCount() const { return m_RepeatCount; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyPressedEvent: " << m_KeyCode << " (" << m_RepeatCount << " repeats)";
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyPressed)

	private:
		/**
		 * When a key is pressed, wait a bit and then keep repeating pressed event until the key is released.
		 *
		 * Imagine you press and hold the "A" key in Visual Studio, then what happens...
		 */
		U16 m_RepeatCount;

	};

	class KeyReleasedEvent : public KeyEvent
	{
	public:
		KeyReleasedEvent(GLFWwindow* window, const KeyCode keycode)
			: KeyEvent(window, keycode) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyReleasedEvent: " << m_KeyCode;
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyReleased)

	};

	/** Event used for character input. */
	class KeyTypedEvent : public KeyEvent
	{
	public:
		KeyTypedEvent(GLFWwindow* window, const KeyCode keycode)
			: KeyEvent(window, keycode) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyTypedEvent: " << m_KeyCode;
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyTyped)

	};
}

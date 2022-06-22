#pragma once

#include <string>
#include <functional>
#include <ostream>

#include "Engine/Core/CoreMacros.h"

struct GLFWwindow;

namespace ZeoEngine {

	// Events in ZeoEngine are currently blocking, meaning when an event occurs it
	// immediately gets dispatched and must be dealt with right then an there.
	// For the future, a better strategy might be to buffer events in an event
	// bus and process them during the "event" part of the update stage.

	enum class EventType
	{
		None = 0,
		// Application events
		WindowClose, WindowResize, WindowFocusChanged, WindowMoved, WindowFileDropped,
		AppTick, AppUpdate, AppRender,
		// Keyboard events
		KeyPressed, KeyReleased, KeyTyped,
		// Mouse events
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
	};

	enum EventCategory
	{
		EventCategoryNone = 0,
		EventCategoryApplication	= ZE_BIT(0),
		EventCategoryInput			= ZE_BIT(1),
		EventCategoryKeyboard		= ZE_BIT(2),
		EventCategoryMouse			= ZE_BIT(3),
		EventCategoryMouseButton	= ZE_BIT(4)
	};

#define EVENT_CLASS_TYPE(type) static EventType GetStaticType() { return EventType::##type; }\
								virtual EventType GetEventType() const override { return GetStaticType(); }\
								virtual const char* GetName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) virtual int GetCategoryFlags() const override { return category; }

	class Event
	{
	public:
		explicit Event(GLFWwindow* window)
			: m_Window(window) {}

		GLFWwindow* GetWindow() const { return m_Window; }

		virtual EventType GetEventType() const = 0;
		virtual const char* GetName() const = 0;
		virtual int GetCategoryFlags() const = 0;
		virtual std::string ToString() const { return GetName(); }

		bool IsInCategory(EventCategory category)
		{
			return GetCategoryFlags() & category;
		}

	public:
		/**
		 * When events are dispatched to various layers, if an event is handled, it will not propagate any further.
		 *
		 * Consider there is a button on the screen, the mouse is clicked and has fallen within the bounds of the button,
		 * that event has been handled and we want to consume that event so that the layer underneath it which might be the game world
		 * will not receive a click event.
		 */
		bool m_bHandled = false;

	private:
		GLFWwindow* m_Window = nullptr;
	};

	class EventDispatcher
	{
	public:
		EventDispatcher(Event& event)
			: m_Event(event)
		{
		}

		// F will be deduced by the compiler
		template<typename T, typename F>
		bool Dispatch(const F& func)
		{
			if (m_Event.GetEventType() == T::GetStaticType())
			{
				m_Event.m_bHandled |= func(static_cast<T&>(m_Event));
				return true;
			}
			return false;
		}

	private:
		Event& m_Event;

	};

	inline std::ostream& operator<<(std::ostream& os, const Event& e)
	{
		return os << e.ToString();
	}

}

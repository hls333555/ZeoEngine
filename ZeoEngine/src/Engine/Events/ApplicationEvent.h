#pragma once

#include <sstream>

#include "Engine/Events/Event.h"

namespace ZeoEngine {

	class WindowResizeEvent : public Event
	{
	public:
		WindowResizeEvent(GLFWwindow* window, unsigned int width, unsigned int height)
			: Event(window), m_Width(width), m_Height(height) {}

		unsigned int GetWidth() const { return m_Width; }
		unsigned int GetHeight() const { return m_Height; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "WindowResizeEvent: " << m_Width << ", " << m_Height;
			return ss.str();
		}

		EVENT_CLASS_TYPE(WindowResize)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)

	private:
		unsigned int m_Width, m_Height;

	};

	class WindowCloseEvent : public Event
	{
	public:
		using Event::Event;

		EVENT_CLASS_TYPE(WindowClose)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)

	};

	class WindowFocusChangedEvent : public Event
	{
	public:
		WindowFocusChangedEvent(GLFWwindow* window, bool bFocused)
			: Event(window), m_bFocused(bFocused) {}

		bool IsFocused() const { return m_bFocused; }

		EVENT_CLASS_TYPE(WindowFocusChanged)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)

	private:
		bool m_bFocused;
	};

	class WindowFileDroppedEvent : public Event
	{
	public:
		WindowFileDroppedEvent(GLFWwindow* window, I32 count, const char** paths)
			: Event(window)
		{
			m_Paths.reserve(count);
			for (I32 i = 0; i < count; ++i)
			{
				m_Paths.emplace_back(paths[i]);
			}
		}

		const auto& GetPaths() const { return m_Paths; }

		EVENT_CLASS_TYPE(WindowFileDropped)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)

	private:
		std::vector<std::string> m_Paths;

	};

	class AppTickEvent : public Event
	{
	public:
		using Event::Event;

		EVENT_CLASS_TYPE(AppTick)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)

	};

	class AppUpdateEvent : public Event
	{
	public:
		using Event::Event;

		EVENT_CLASS_TYPE(AppUpdate)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)

	};

	class AppRenderEvent : public Event
	{
	public:
		using Event::Event;

		EVENT_CLASS_TYPE(AppRender)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)

	};
}

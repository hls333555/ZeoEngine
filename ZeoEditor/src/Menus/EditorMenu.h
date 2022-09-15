#pragma once

#include <string>
#include <vector>

#include "Engine/Core/Core.h"
#include "Engine/Events/Event.h"

namespace ZeoEngine {

	class Editor;
	class MenuItemBase;

	class EditorMenu
	{
	public:
		explicit EditorMenu(std::string menuName);

		void OnImGuiRender() const;
		void OnEvent(Event& e) const;

		void SetEnabled(bool bEnabled) { m_bEnabled = bEnabled; }

		template<typename T, typename ... Args>
		EditorMenu& MenuItem(Args&& ... args)
		{
			Scope<T> menuItem = CreateScope<T>(std::forward<Args>(args)...);
			m_MenuItems.emplace_back(std::move(menuItem));
			return *this;
		}

	private:
		void RenderMenuItems() const;

	private:
		std::string m_MenuName;
		bool m_bEnabled = true;
		std::vector<Scope<MenuItemBase>> m_MenuItems;
	};

}

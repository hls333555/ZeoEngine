#pragma once

#include <string>
#include <vector>

#include "Engine/Core/Core.h"
#include "Engine/Events/Event.h"

namespace ZeoEngine {

	class EditorBase;
	class MenuItemBase;

	class EditorMenu
	{
	public:
		EditorMenu() = delete;
		EditorMenu(std::string menuName, const Weak<EditorBase>& contextEditor);

		void OnImGuiRender();
		void OnEvent(Event& e);

		void SetEnabled(bool bEnabled) { m_bEnabled = bEnabled; }

		template<typename T, typename ... Args>
		EditorMenu& MenuItem(Args&& ... args)
		{
			Ref<T> menuItem = CreateRef<T>(m_ContextEditor, std::forward<Args>(args)...);
			m_MenuItems.emplace_back(menuItem);
			return *this;
		}

	private:
		void RenderMenuItems();

	private:
		std::string m_MenuName;
		Weak<EditorBase> m_ContextEditor;
		bool m_bEnabled = true;
		std::vector<Ref<MenuItemBase>> m_MenuItems;
	};

}

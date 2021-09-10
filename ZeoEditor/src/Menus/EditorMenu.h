#pragma once

#include <string>
#include <vector>

#include "Engine/Events/Event.h"

namespace ZeoEngine {

	class EditorBase;
	class MenuItemBase;

	class EditorMenu
	{
		friend class MenuManager;

	private:
		EditorMenu() = delete;
		EditorMenu(const std::string& menuName, EditorBase* owningEditor);
		~EditorMenu();

	public:
		void OnImGuiRender();
		void OnEvent(Event& e);

		void SetEnabled(bool bEnabled) { m_bEnabled = bEnabled; }

		template<typename T, typename ... Args>
		EditorMenu& MenuItem(Args&& ... args)
		{
			T* menuItem = new T(m_OwningEditor, std::forward<Args>(args)...);
			m_MenuItems.emplace_back(menuItem);
			return *this;
		}

	private:
		void RenderMenuItems();

	private:
		std::string m_MenuName;
		EditorBase* m_OwningEditor = nullptr;
		bool m_bEnabled = true;
		std::vector<MenuItemBase*> m_MenuItems;
	};

}

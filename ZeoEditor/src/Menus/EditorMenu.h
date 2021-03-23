#pragma once

#include <string>
#include <vector>

#include "Engine/Events/Event.h"

namespace ZeoEngine {

	class DockspaceBase;
	class MenuItemBase;

	class EditorMenu
	{
		friend class MenuManager;

	private:
		EditorMenu() = delete;
		EditorMenu(const std::string& menuName, DockspaceBase* context);
		~EditorMenu();

	public:
		void OnImGuiRender();
		void OnEvent(Event& e);

		template<typename T = DockspaceBase>
		T* GetContext() { return dynamic_cast<T*>(m_Context); }

		void SetEnabled(bool bEnabled) { m_bEnabled = bEnabled; }

		template<typename T, typename ... Args>
		EditorMenu& MenuItem(Args&& ... args)
		{
			T* menuItem = new T(m_Context, std::forward<Args>(args)...);
			m_MenuItems.emplace_back(menuItem);
			return *this;
		}

	private:
		void RenderMenuItems();

	private:
		std::string m_MenuName;
		DockspaceBase* m_Context = nullptr;
		bool m_bEnabled = true;
		std::vector<MenuItemBase*> m_MenuItems;
	};

}

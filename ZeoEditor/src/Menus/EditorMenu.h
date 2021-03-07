#pragma once

#include <string>
#include <vector>

#include "Engine/Events/Event.h"

namespace ZeoEngine {

	class EditorMenuItem;
	class EditorDockspace;

	class EditorMenu
	{
		friend class MenuManager;

	private:
		EditorMenu() = delete;
		EditorMenu(const std::string& menuName, EditorDockspace* context);
	public:
		~EditorMenu();

		void OnImGuiRender();
		void OnEvent(Event& e);

		template<typename T = EditorDockspace>
		T* GetContext() { return dynamic_cast<T*>(m_Context); }
		void SetEnabled(bool bEnabled) { m_bEnabled = bEnabled; }

		template<typename T, typename ... Args>
		EditorMenu& MenuItem(Args&& ... args)
		{
			T* menuItem = new T(this, std::forward<Args>(args)...);
			m_MenuItems.emplace_back(menuItem);
			return *this;
		}

	private:
		void RenderMenuItems();

	private:
		std::string m_MenuName;
		EditorDockspace* m_Context;
		bool m_bEnabled = true;
		std::vector<EditorMenuItem*> m_MenuItems;
	};

}

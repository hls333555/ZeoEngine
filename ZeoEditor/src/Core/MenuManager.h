#pragma once

#include "Engine/Events/Event.h"

namespace ZeoEngine {

	class EditorMenu;
	class EditorBase;

	class MenuManager
	{
		friend class DockspaceBase;

	private:
		MenuManager() = default;
		~MenuManager();

	public:
		EditorMenu& CreateMenu(const std::string& menuName, EditorBase* owningEditor);

		void OnImGuiRender(bool bIsMainMenu);
		void OnEvent(Event& e);

	private:
		void RenderMainMenuBar();
		void RenderMenuBar();
		void RenderMenus();

	private:
		std::vector<EditorMenu*> m_Menus;
	};

}

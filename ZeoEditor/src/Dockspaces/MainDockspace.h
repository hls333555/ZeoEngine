#pragma once

#include "Dockspaces/EditorDockspace.h"

namespace ZeoEngine {

	class MainDockspace : public EditorDockspace
	{
	public:
		using EditorDockspace::EditorDockspace;

		virtual void OnAttach() override;

		Entity& GetSeletedEntity() { return m_ContextEntity; }

	private:
		virtual void BuildDockWindows(ImGuiID dockspaceID) override;

	};

}

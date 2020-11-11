#pragma once

#include "Dockspaces/EditorDockspace.h"

namespace ZeoEngine {

	class ParticleEditorDockspace : public EditorDockspace
	{
	public:
		using EditorDockspace::EditorDockspace;

		virtual void OnAttach() override;

	private:
		virtual void BuildDockWindows(ImGuiID dockspaceID) override;

	private:
		entt::delegate<void()> m_ParticleInitDel;

	};

}

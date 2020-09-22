#pragma once

#include "Dockspaces/EditorDockspace.h"

#include "Engine/GameFramework/Entity.h"

namespace ZeoEngine {

	class MainDockspace : public EditorDockspace
	{
		friend class SceneOutlinePanel;
		friend class EntityInspectorPanel;

	public:
		explicit MainDockspace(const std::string& dockspaceName,
			float dockspaceRounding = 0.0f, float dockspaceBorderSize = 0.0f, ImVec2 dockspacePadding = ImVec2(0.0f, 0.0f),
			ImGuiWindowFlags dockspaceWindowFlags = ImGuiWindowFlags_MenuBar,
			ImVec2 dockspacePos = ImVec2(0.0f, 0.0f), ImVec2 dockspaceSize = ImVec2(700.0f, 500.0f));

		virtual void OnAttach() override;

	private:
		virtual void BuildDockWindows(ImGuiID dockspaceID) override;

	private:
		Entity m_SelectedEntity;

	};

}

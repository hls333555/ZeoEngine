#pragma once

#include "Panels/EditorPanel.h"

#include "Engine/GameFramework/Entity.h"

namespace ZeoEngine {

	class EditorDockspace;

	class SceneOutlinePanel : public ScenePanel
	{
	public:
		using ScenePanel::ScenePanel;

		virtual void OnImGuiRender() override;

	private:
		void DrawEntityNode(Entity entity);

	private:
		Entity m_SelectedEntity;
	};

}

#pragma once

#include "Panels/PanelBase.h"

#include "Engine/GameFramework/Entity.h"
#include "Engine/ImGui/TextFilter.h"

namespace ZeoEngine {

	class EditorPreviewWorldBase;

	class LevelOutlinePanel : public PanelBase
	{
	public:
		explicit LevelOutlinePanel(std::string panelName);

	private:
		virtual void ProcessRender() override;

		void DrawEntityNode(Entity entity) const;

	private:
		Weak<EditorPreviewWorldBase> m_EditorWorld;

		TextFilter m_Filter;

	};

}

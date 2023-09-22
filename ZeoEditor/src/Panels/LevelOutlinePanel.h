#pragma once

#include "Panels/PanelBase.h"

#include "Engine/GameFramework/Entity.h"
#include "Widgets/TextFilter.h"

namespace ZeoEngine {

	class EditorPreviewWorldBase;

	class LevelOutlinePanel : public PanelBase
	{
	public:
		explicit LevelOutlinePanel(std::string panelName);

	private:
		virtual void ProcessRender() override;

		void DrawEntityNode(Entity entity);
		bool DoesAnyChildPassFilter(const Entity& entity) const;
		void ToggleEntityVisibilityRecursively(const Entity& entity, bool bHide) const;
		void ToggleAllEntitiesVisibility(bool bHide) const;

	private:
		EditorPreviewWorldBase* m_EditorWorld = nullptr;

		TextFilter m_Filter;

		bool m_bAnyEntityVisible = true;

	};

}

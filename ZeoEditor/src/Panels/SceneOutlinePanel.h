#pragma once

#include "Panels/PanelBase.h"

#include "Engine/GameFramework/Entity.h"
#include "Engine/ImGui/TextFilter.h"

namespace ZeoEngine {

	class SceneOutlinePanel : public PanelBase
	{
	public:
		using PanelBase::PanelBase;

	private:
		virtual void ProcessRender() override;

		void DrawEntityNode(Entity entity);

	private:
		TextFilter m_Filter;

	};

}

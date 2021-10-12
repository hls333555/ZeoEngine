#pragma once

#include "Panels/EditorViewPanelBase.h"

namespace ZeoEngine {

	class ParticleEditorViewPanel : public EditorViewPanelBase
	{
	public:
		using EditorViewPanelBase::EditorViewPanelBase;

	private:
		virtual void ProcessRender() override;

		virtual void RenderToolbar() override;

	};

}

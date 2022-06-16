#pragma once

#include "Panels/EditorViewPanelBase.h"

namespace ZeoEngine {

	class TextureEditorViewPanel : public EditorViewPanelBase
	{
	public:
		using EditorViewPanelBase::EditorViewPanelBase;

	private:
		virtual void ProcessUpdate(DeltaTime dt) override {}
		virtual void ProcessRender() override;

	};

}

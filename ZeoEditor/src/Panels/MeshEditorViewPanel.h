#pragma once

#include "Panels/EditorViewPanelBase.h"

namespace ZeoEngine {

	class MeshEditorViewPanel : public EditorViewPanelBase
	{
	public:
		using EditorViewPanelBase::EditorViewPanelBase;

	private:
		virtual void ProcessRender() override;

	};

}

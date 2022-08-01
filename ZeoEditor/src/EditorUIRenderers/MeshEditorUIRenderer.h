#pragma once

#include "EditorUIRenderers/EditorUIRendererBase.h"

namespace ZeoEngine {

	class MeshEditorUIRenderer : public EditorUIRendererBase
	{
	public:
		using EditorUIRendererBase::EditorUIRendererBase;

		virtual void OnAttach() override;

	private:
		virtual void BuildDockWindows(ImGuiID dockspaceID) override;

	};

}

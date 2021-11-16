#include "Panels/MaterialEditorViewPanel.h"

namespace ZeoEngine {

	void MaterialEditorViewPanel::ProcessRender()
	{
		// Get default available region before drawing any widgets
		const ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

		EditorViewPanelBase::ProcessRender();

	}

}

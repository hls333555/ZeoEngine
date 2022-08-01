#include "Panels/MeshEditorViewPanel.h"

namespace ZeoEngine {

	void MeshEditorViewPanel::ProcessRender()
	{
		// Get default available region before drawing any widgets
		const ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

		EditorViewPanelBase::ProcessRender();

	}

}

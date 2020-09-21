#include "Panels/ParticleViewportPanel.h"

#include <imgui.h>

namespace ZeoEngine {

	void ParticleViewportPanel::OnImGuiRender()
	{
		if (!m_bShow) return;

		SceneViewportPanel::OnImGuiRender();

		if (ImGui::Begin(m_PanelName.c_str(), &m_bShow))
		{

		}
		ImGui::End();
	}

}

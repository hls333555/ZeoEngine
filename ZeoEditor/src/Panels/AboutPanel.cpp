#include "Panels/AboutPanel.h"

#include "Engine/ImGui/MyImGui.h"

namespace ZeoEngine {

	void AboutPanel::OnAttach()
	{
		m_LogoTexture = Texture2D::Create("assets/textures/Logo.png");
	}

	void AboutPanel::OnImGuiRender()
	{
		if (!m_bShow) return;

		EditorPanel::OnImGuiRender();

		if (ImGui::Begin(m_PanelName.c_str(), &m_bShow))
		{
			ImGui::TextCentered("ZeoEngine 0.1");
			ImGui::TextCentered("Created by SanSan");
			ImGui::TextCentered("https://github.com/hls333555/");
			const float logoSize = 100.0f;
			// Center the logo
			ImGui::Indent((ImGui::GetWindowSize().x - logoSize) / 2.0f);
			ImGui::Image(m_LogoTexture->GetTexture(),
				ImVec2(logoSize, logoSize),
				ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
		}
		ImGui::End();
	}

}

#include "Panels/AboutPanel.h"

#include "Engine/ImGui/MyImGui.h"

namespace ZeoEngine {

	void AboutPanel::OnAttach()
	{
		m_LogoTexture = Texture2D::Create("assets/textures/Logo.png");
	}

	void AboutPanel::ProcessRender()
	{
		ImGui::TextCentered("ZeoEngine 0.1");
		ImGui::TextCentered("Created by SanSan");
		ImGui::TextCentered("https://github.com/hls333555/");
		const float logoSize = 100.0f;
		// Center the logo
		ImGui::Indent((ImGui::GetContentRegionAvail().x - logoSize) * 0.5f);
		ImGui::Image(m_LogoTexture->GetTextureID(),
			{ logoSize, logoSize },
			{ 0.0f, 1.0f }, { 1.0f, 0.0f });
	}

}

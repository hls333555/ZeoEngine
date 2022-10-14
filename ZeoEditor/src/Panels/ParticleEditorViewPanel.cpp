#include "Panels/ParticleEditorViewPanel.h"

#include <IconsFontAwesome5.h>

#include "Engine/GameFramework/Components.h"
#include "Worlds/EditorPreviewWorldBase.h"

namespace ZeoEngine {

	void ParticleEditorViewPanel::ProcessRender()
	{
		// Get default available region before drawing any widgets
		const ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

		ViewPanelBase::ProcessRender();

		const auto& psInstance = GetEditorWorld()->GetContextEntity().GetComponent<ParticleSystemDetailComponent>().Instance;

		// Display particle count at the top right corner
		{
			char particleCount[16];
			_itoa_s(psInstance->m_ActiveParticleCount, particleCount, 10);
			const float particleCountWidth = ImGui::CalcTextSize(particleCount).x;
			ImGui::SameLine(contentRegionAvailable.x - particleCountWidth);
			ImGui::TextColored({ 1.0f, 1.0f, 0.0f, 1.0f }, particleCount);
		}

		// Display "Completed" text at the bottom center
		if (psInstance->m_bSystemComplete)
		{
			static const char* completedStr = "Completed";
			static const float completedWidth = ImGui::CalcTextSize(completedStr).x;
			ImGui::SetCursorPos({ (contentRegionAvailable.x - completedWidth) * 0.5f + ImGui::GetFramePadding().x, contentRegionAvailable.y });
			ImGui::Text(completedStr);
		}
	}
	
	void ParticleEditorViewPanel::RenderToolbar()
	{
		const auto& psInstance = GetEditorWorld()->GetContextEntity().GetComponent<ParticleSystemDetailComponent>().Instance;

		// Place buttons at window center
		ImGui::Indent(ImGui::GetContentRegionAvail().x * 0.5f - ImGui::GetFrameHeightWithSpacing());

		// Toggle pause / resume
		if (ImGui::TransparentButton(psInstance->IsPause() ? ICON_FA_PLAY : ICON_FA_PAUSE))
		{
			psInstance->TogglePause();
		}

		ImGui::SameLine();

		// Resimulate
		if (ImGui::TransparentButton(ICON_FA_REDO_ALT))
		{
			psInstance->Resimulate();
		}
	}

}

#include "Panels/ParticleViewPanel.h"

#include <IconsFontAwesome5.h>

#include "Engine/GameFramework/Components.h"
#include "Editors/EditorBase.h"

namespace ZeoEngine {

	void ParticleViewPanel::OnAttach()
	{
		ViewPanelBase::OnAttach();

		GetContextEditor()->m_PostSceneCreate.connect<&ParticleViewPanel::CreatePreviewParticle>(this);
	}

	void ParticleViewPanel::ProcessRender()
	{
		// Get default available region before drawing any widgets
		const ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

		ViewPanelBase::ProcessRender();

		const auto& psInstance = m_PreviewParticleEntity.GetComponent<ParticleSystemPreviewComponent>().Instance;

		// Display particle count at the top right corner of Particle View window
		{
			char particleCount[16];
			_itoa_s(psInstance->m_ActiveParticleCount, particleCount, 10);
			const float particleCountWidth = ImGui::CalcTextSize(particleCount).x;
			ImGui::SameLine(contentRegionAvailable.x - particleCountWidth);
			ImGui::TextColored({ 1.0f, 1.0f, 0.0f, 1.0f }, particleCount);
		}

		// Display "Completed" text at the bottom center of Particle View window
		if (psInstance->m_bSystemComplete)
		{
			static const char* completedStr = "Completed";
			static const float completedWidth = ImGui::CalcTextSize(completedStr).x;
			ImGui::SetCursorPos({ (contentRegionAvailable.x - completedWidth) * 0.5f + ImGui::GetFramePadding().x, contentRegionAvailable.y });
			ImGui::Text(completedStr);
		}
	}

	void ParticleViewPanel::CreatePreviewParticle(bool bIsFromOpen)
	{
		m_PreviewParticleEntity = GetContextEditor()->GetScene()->CreateEntity("Preview Particle", true);
		m_PreviewParticleEntity.AddComponent<ParticleSystemPreviewComponent>();
		GetContextEditor()->SetContextEntity(m_PreviewParticleEntity);
		if (!bIsFromOpen)
		{
			CreateDefaultParticleSystem();
		}
	}

	void ParticleViewPanel::CreateDefaultParticleSystem()
	{
		auto& particlePreviewComp = m_PreviewParticleEntity.GetComponent<ParticleSystemPreviewComponent>();
		particlePreviewComp.Template->Lifetime.SetRandom(0.75f, 1.5f);
		particlePreviewComp.Template->SpawnRate.SetConstant(30.0f);
		particlePreviewComp.Template->InitialRotation.SetRandom(glm::vec3{ 0.0f }, { 0.0f, 0.0f, 360.0f });
		particlePreviewComp.Template->RotationRate.SetRandom(glm::vec3{ 0.0f, 0.0f, 10.0f }, glm::vec3{ 0.0f, 0.0f, 50.0f });
		particlePreviewComp.Template->InitialVelocity.SetRandom({ -0.5f, 0.5f, 0.0f }, { 0.5f, 2.0f, 0.0f });
		particlePreviewComp.Template->SizeBegin.SetRandom({ 0.1f, 0.1f, 0.0f }, { 0.2f, 0.2f, 0.0f });
		particlePreviewComp.Template->SizeEnd.SetConstant(glm::vec3{ 0.0f });
		particlePreviewComp.Template->ColorBegin.SetConstant(glm::vec4{ 1.0f });
		particlePreviewComp.Template->ColorEnd.SetConstant(glm::vec4{ 0.0f });
		ParticleSystemInstance::Create(particlePreviewComp);
	}
	
	void ParticleViewPanel::RenderToolbar()
	{
		const auto& psInstance = m_PreviewParticleEntity.GetComponent<ParticleSystemPreviewComponent>().Instance;

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

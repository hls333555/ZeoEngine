#include "Panels/ParticleViewportPanel.h"

#include <IconsFontAwesome5.h>

#include "Engine/GameFramework/Components.h"
#include "Dockspaces/DockspaceBase.h"

namespace ZeoEngine {

	void ParticleViewportPanel::OnAttach()
	{
		SceneViewportPanel::OnAttach();

		CreatePreviewParticle();
		GetContext()->m_PostSceneCreate.connect<&ParticleViewportPanel::CreatePreviewParticle>(this);
	}

	void ParticleViewportPanel::ProcessRender()
	{
		// Get default available region before drawing any widgets
		const ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

		SceneViewportPanel::ProcessRender();

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

	void ParticleViewportPanel::Snapshot(const std::string& imageName, uint32_t imageWidth)
	{
		SceneViewportPanel::Snapshot(imageName, imageWidth);

		m_PreviewParticleEntity.GetComponent<ParticleSystemPreviewComponent>().Template->UpdatePreviewThumbnail(imageName);
	}

	void ParticleViewportPanel::CreatePreviewParticle(bool bIsFromOpenScene)
	{
		m_PreviewParticleEntity = GetContext()->GetScene()->CreateEntity("Preview Particle", true);
		m_PreviewParticleEntity.AddComponent<ParticleSystemPreviewComponent>();
		GetContext()->SetContextEntity(m_PreviewParticleEntity);
		if (!bIsFromOpenScene)
		{
			CreateDefaultParticleSystem();
		}
	}

	void ParticleViewportPanel::CreateDefaultParticleSystem()
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
	
	void ParticleViewportPanel::RenderToolbar()
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

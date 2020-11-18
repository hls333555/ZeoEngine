#include "Panels/ParticleViewportPanel.h"

#include "Engine/GameFramework/Components.h"
#include "Dockspaces/EditorDockspace.h"

namespace ZeoEngine {

	void ParticleViewportPanel::OnAttach()
	{
		SceneViewportPanel::OnAttach();

		CreatePreviewParticle();
		GetContext()->m_OnSceneCreate.connect<&ParticleViewportPanel::CreatePreviewParticle>(this);
	}

	void ParticleViewportPanel::CreatePreviewParticle(bool bIsFromOpenScene)
	{
		Entity previewParticleEntity = GetScene()->CreateEntity("Preview Particle", true);
		m_ParticlePreviewComp = &previewParticleEntity.AddComponent<ParticleSystemPreviewComponent>();
		GetContext()->SetContextEntity(previewParticleEntity);
		if (!bIsFromOpenScene)
		{
			CreateDefaultParticleSystem();
		}
	}

	void ParticleViewportPanel::Snapshot(const std::string& imageName, uint32_t imageWidth)
	{
		SceneViewportPanel::Snapshot(imageName, imageWidth);

		m_ParticlePreviewComp->Template->UpdatePreviewThumbnail(imageName);
	}

	void ParticleViewportPanel::CreateDefaultParticleSystem()
	{
		m_ParticlePreviewComp->Template->Lifetime.SetRandom(0.75f, 1.5f);
		m_ParticlePreviewComp->Template->SpawnRate.SetConstant(30.0f);
		m_ParticlePreviewComp->Template->InitialRotation.SetRandom(glm::vec3{ 0.0f }, { 0.0f, 0.0f, 360.0f });
		m_ParticlePreviewComp->Template->RotationRate.SetRandom(glm::vec3{ 0.0f, 0.0f, 10.0f }, glm::vec3 { 0.0f, 0.0f, 50.0f });
		m_ParticlePreviewComp->Template->InitialVelocity.SetRandom({ -0.5f, 0.5f, 0.0f }, { 0.5f, 2.0f, 0.0f });
		m_ParticlePreviewComp->Template->SizeBegin.SetRandom({ 0.1f, 0.1f, 0.0f }, { 0.2f, 0.2f, 0.0f });
		m_ParticlePreviewComp->Template->SizeEnd.SetConstant(glm::vec3{ 0.0f });
		m_ParticlePreviewComp->Template->ColorBegin.SetConstant(glm::vec4{ 1.0f });
		m_ParticlePreviewComp->Template->ColorEnd.SetConstant(glm::vec4{ 0.0f });
		// Old template's particle system reference is cleared on old scene's destruction, so just pass null here
		m_ParticlePreviewComp->CreateParticleSystem({});
	}

	void ParticleViewportPanel::RenderPanel()
	{
		SceneViewportPanel::RenderPanel();

		ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

		const auto& ps = m_ParticlePreviewComp->ParticleSystemRuntime;

		// Display "Completed" text at the top center of Particle View window
		if (ps->m_bSystemComplete)
		{
			static const float completedWidth = ImGui::CalcTextSize("Completed").x;
			ImGui::Indent((contentRegionAvailable.x - completedWidth) * 0.5f);
			ImGui::Text("Complete");
		}
		// Display particle count at the top right corner of Particle View window
		{
			char particleCount[16];
			_itoa_s(ps->m_ActiveParticleCount, particleCount, 10);
			const float particleCountWidth = ImGui::CalcTextSize(particleCount).x;
			ImGui::SameLine(contentRegionAvailable.x - particleCountWidth);
			ImGui::TextColored({ 1.0f, 1.0f, 0.0f, 1.0f }, particleCount);
		}
	}

}

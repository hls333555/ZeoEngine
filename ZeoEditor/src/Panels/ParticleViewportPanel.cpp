#include "Panels/ParticleViewportPanel.h"

#include "Engine/GameFramework/Components.h"
#include "Dockspaces/EditorDockspace.h"

namespace ZeoEngine {

	void ParticleViewportPanel::OnAttach()
	{
		SceneViewportPanel::OnAttach();

		CreatePreviewParticle();
	}

	void ParticleViewportPanel::CreatePreviewParticle(bool bIsFromOpenScene)
	{
		Entity previewParticleEntity = GetScene()->CreateEntity("Preview Particle", true);
		ParticleSystemPreviewComponent& particleComp = previewParticleEntity.AddComponent<ParticleSystemPreviewComponent>();
		GetContext()->SetContextEntity(previewParticleEntity);
		if (!bIsFromOpenScene)
		{
			CreateDefaultParticleSystem(particleComp);
		}
	}

	void ParticleViewportPanel::CreateDefaultParticleSystem(ParticleSystemPreviewComponent& particleComp)
	{
		particleComp.Template->Lifetime.SetRandom(0.75f, 1.5f);
		particleComp.Template->SpawnRate.SetConstant(30.0f);
		particleComp.Template->InitialRotation.SetRandom(glm::vec3{ 0.0f }, { 0.0f, 0.0f, 360.0f });
		particleComp.Template->RotationRate.SetRandom(glm::vec3{ 0.0f, 0.0f, 10.0f }, glm::vec3 { 0.0f, 0.0f, 50.0f });
		particleComp.Template->InitialVelocity.SetRandom({ -0.5f, 0.5f, 0.0f }, { 0.5f, 2.0f, 0.0f });
		particleComp.Template->SizeBegin.SetRandom({ 0.1f, 0.1f, 0.0f }, { 0.2f, 0.2f, 0.0f });
		particleComp.Template->SizeEnd.SetConstant(glm::vec3{ 0.0f });
		particleComp.Template->ColorBegin.SetConstant(glm::vec4{ 1.0f });
		particleComp.Template->ColorEnd.SetConstant(glm::vec4{ 0.0f });
		particleComp.CreateParticleSystem();
	}

	void ParticleViewportPanel::RenderPanel()
	{
		SceneViewportPanel::RenderPanel();

		ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

		const auto& ps = GetContext()->GetContextEntity().GetComponent<ParticleSystemPreviewComponent>().ParticleSystemRuntime;

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

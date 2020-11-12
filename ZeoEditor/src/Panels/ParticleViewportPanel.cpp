#include "Panels/ParticleViewportPanel.h"

#include "Engine/GameFramework/Components.h"
#include "Dockspaces/EditorDockspace.h"

namespace ZeoEngine {

	void ParticleViewportPanel::OnAttach()
	{
		SceneViewportPanel::OnAttach();

		CreatePreviewParticle();
	}

	void ParticleViewportPanel::CreatePreviewParticle()
	{
		Entity previewParticleEntity = GetScene()->CreateEntity("Preview Particle", true);
		ParticleSystemPreviewComponent& particleComp = previewParticleEntity.AddComponent<ParticleSystemPreviewComponent>();
		GetContext()->SetContextEntity(previewParticleEntity);
		CreateDefaultParticleSystem(particleComp);
	}

	void ParticleViewportPanel::CreateDefaultParticleSystem(ParticleSystemPreviewComponent& particleComp)
	{
		particleComp.Template->Lifetime.SetRandom(0.75f, 1.5f);
		particleComp.Template->SpawnRate.SetConstant(30.0f);
		particleComp.Template->InitialRotation.SetRandom(0.0f, 360.0f);
		particleComp.Template->RotationRate.SetRandom(10.0f, 50.0f);
		particleComp.Template->InitialVelocity.SetRandom({ -0.5f, 0.5f }, { 0.5f, 2.0f });
		particleComp.Template->SizeBegin.SetRandom({ 0.1f, 0.1f }, { 0.2f, 0.2f });
		particleComp.Template->SizeEnd.SetConstant({ 0.0f, 0.0f });
		particleComp.Template->ColorBegin.SetConstant({ 1.0f, 1.0f, 1.0f, 1.0f });
		particleComp.Template->ColorEnd.SetConstant({ 0.0f, 0.0f, 0.0f, 0.0f });
		particleComp.ParticleSystemRuntime = CreateRef<ParticleSystem>(particleComp.Template);
	}

	void ParticleViewportPanel::RenderPanel()
	{
		SceneViewportPanel::RenderPanel();


	}

}

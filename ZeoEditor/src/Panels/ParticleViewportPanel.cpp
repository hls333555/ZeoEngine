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
		previewParticleEntity.AddComponent<ParticleSystemDetailComponent>();
		GetContext()->SetContextEntity(previewParticleEntity);
	}

	void ParticleViewportPanel::RenderPanel()
	{
		SceneViewportPanel::RenderPanel();


	}

}

#include "Panels/ParticleViewportPanel.h"

#include "Engine/GameFramework/Components.h"
#include "Dockspaces/EditorDockspace.h"

namespace ZeoEngine {

	void ParticleViewportPanel::OnAttach()
	{
		SceneViewportPanel::OnAttach();

		Entity defaultParticleEntity = GetScene()->CreateEmptyEntity();
		defaultParticleEntity.AddComponent<ParticleSystemPreviewComponent>();
		GetContext()->SetContextEntity(defaultParticleEntity);
	}

	void ParticleViewportPanel::RenderPanel()
	{
		SceneViewportPanel::RenderPanel();


	}

}

#include "Scenes/ParticleEditorScene.h"

#include "Engine/GameFramework/Systems.h"
#include "Engine/Renderer/Renderer2D.h"

namespace ZeoEngine {

	ParticleEditorScene::ParticleEditorScene() = default;

	void ParticleEditorScene::OnAttach()
	{
		RegisterSystem<ParticlePreviewUpdateSystem>(shared_from_this());
	}

	void ParticleEditorScene::PostLoad()
	{
		m_Registry.view<ParticleSystemPreviewComponent>().each([](auto entity, auto& particlePreviewComp)
		{
			ParticleSystemInstance::Create(particlePreviewComp);
		});
	}

}

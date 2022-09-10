#include "Scenes/ParticlePreviewScene.h"

#include "Engine/GameFramework/Components.h"
#include "Engine/GameFramework/Systems.h"

namespace ZeoEngine {

	void ParticlePreviewScene::OnAttach(const Ref<WorldBase>& world)
	{
		RegisterSystem<ParticlePreviewUpdateSystem>(world);
	}

	void ParticlePreviewScene::PostLoad()
	{
		m_Registry.view<ParticleSystemPreviewComponent>().each([](auto entity, auto& particlePreviewComp)
		{
			ParticleSystemInstance::Create(particlePreviewComp);
		});
	}

}

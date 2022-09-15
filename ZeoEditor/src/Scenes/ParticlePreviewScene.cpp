#include "Scenes/ParticlePreviewScene.h"

#include "Engine/GameFramework/Components.h"
#include "Engine/GameFramework/Systems.h"

namespace ZeoEngine {

	ParticlePreviewScene::~ParticlePreviewScene()
	{
		ForEachComponentView<ParticleSystemPreviewComponent>([](auto entity, auto& particlePreviewComp)
		{
			if (particlePreviewComp.ParticleTemplateAsset)
			{
				particlePreviewComp.ParticleTemplateAsset->RemoveParticleSystemInstance(particlePreviewComp.Instance);
			}
		});
	}

	void ParticlePreviewScene::OnAttach(const Ref<WorldBase>& world)
	{
		RegisterSystem<ParticlePreviewUpdateSystem>(world);
	}

	void ParticlePreviewScene::PostLoad()
	{
		ForEachComponentView<ParticleSystemPreviewComponent>([](auto entity, auto& particlePreviewComp)
		{
			ParticleSystemInstance::Create(particlePreviewComp);
		});
	}

}

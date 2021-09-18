#include "Scenes/ParticleScene.h"

#include "Engine/GameFramework/Systems.h"
#include "Engine/Renderer/Renderer2D.h"

namespace ZeoEngine {

	ParticleScene::ParticleScene()
	{
		m_ParticlePreviewRenderSystem = CreateScope<ParticlePreviewRenderSystem>(this);
		m_ParticlePreviewRenderSystem->OnCreate();
	}

	ParticleScene::~ParticleScene()
	{
		m_ParticlePreviewRenderSystem->OnDestroy();
	}

	void ParticleScene::OnUpdate(DeltaTime dt)
	{
		m_ParticlePreviewRenderSystem->OnUpdate(dt);
	}

	void ParticleScene::OnRender(const EditorCamera& camera)
	{
		m_ParticlePreviewRenderSystem->OnRender(camera);
	}

	void ParticleScene::PostLoad()
	{
		m_Registry.view<ParticleSystemPreviewComponent>().each([](auto entity, auto& particlePreviewComp)
		{
			ParticleSystemInstance::Create(particlePreviewComp);
		});
	}

}

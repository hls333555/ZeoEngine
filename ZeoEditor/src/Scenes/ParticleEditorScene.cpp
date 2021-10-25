#include "Scenes/ParticleEditorScene.h"

#include "Engine/GameFramework/Systems.h"
#include "Engine/Renderer/Renderer2D.h"

namespace ZeoEngine {

	ParticleEditorScene::ParticleEditorScene()
	{
		m_ParticlePreviewRenderSystem = CreateScope<ParticlePreviewRenderSystem>(this);
		m_ParticlePreviewRenderSystem->OnCreate();
	}

	ParticleEditorScene::~ParticleEditorScene()
	{
		m_ParticlePreviewRenderSystem->OnDestroy();
	}

	void ParticleEditorScene::OnUpdate(DeltaTime dt)
	{
		m_ParticlePreviewRenderSystem->OnUpdate(dt);
	}

	void ParticleEditorScene::OnRender(const EditorCamera& camera)
	{
		m_ParticlePreviewRenderSystem->OnRender(camera);
	}

	void ParticleEditorScene::PostLoad()
	{
		m_Registry.view<ParticleSystemPreviewComponent>().each([](auto entity, auto& particlePreviewComp)
		{
			ParticleSystemInstance::Create(particlePreviewComp);
		});
	}

}

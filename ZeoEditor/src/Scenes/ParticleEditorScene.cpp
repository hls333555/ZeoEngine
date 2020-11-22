#include "Scenes/ParticleEditorScene.h"

#include "Engine/GameFramework/Components.h"

namespace ZeoEngine {

	void ParticleEditorScene::OnUpdate(DeltaTime dt)
	{
		Scene::OnUpdate(dt);

		// Update particle system for particle editor
		m_Registry.view<ParticleSystemPreviewComponent>().each([dt](auto entity, auto& pspc)
		{
			pspc.UpdateParticleSystem(dt);
		});
	}

	void ParticleEditorScene::OnSceneRender()
	{
		// Render particle system for particle editor
		m_Registry.view<ParticleSystemPreviewComponent>().each([](auto entity, auto& pspc)
		{
			pspc.RenderParticleSystem();
		});
	}

	void ParticleEditorScene::OnClenup()
	{
		Scene::OnClenup();

		// Clear particle system reference for particle editor on scene destruction
		m_Registry.view<ParticleSystemPreviewComponent>().each([](auto entity, auto& pspc)
		{
			pspc.RemoveParticleSystemInstance();
		});
	}

}

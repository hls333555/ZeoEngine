#include "Scenes/ParticleEditorScene.h"

#include "Engine/GameFramework/Components.h"
#include "Engine/Renderer/Renderer2D.h"

namespace ZeoEngine {

	void ParticleEditorScene::OnUpdate(DeltaTime dt)
	{
		// Update particle system for particle editor
		m_Registry.view<ParticleSystemPreviewComponent>().each([dt](auto entity, auto& pspc)
		{
			pspc.UpdateParticleSystem(dt);
		});
	}

	void ParticleEditorScene::OnRender(const EditorCamera& camera)
	{
		Renderer2D::BeginScene(camera);
		{
			// Render particle system for particle editor
			m_Registry.view<ParticleSystemPreviewComponent>().each([](auto entity, auto& pspc)
			{
				pspc.RenderParticleSystem();
			});
		}
		Renderer2D::EndScene();
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

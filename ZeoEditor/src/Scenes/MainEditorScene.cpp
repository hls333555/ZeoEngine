#include "Scenes/MainEditorScene.h"

#include "Engine/GameFramework/Components.h"
#include "Engine/Renderer/Renderer2D.h"

namespace ZeoEngine {

	void MainEditorScene::OnUpdate(DeltaTime dt)
	{
		Scene::OnUpdate(dt);

		// Update particle system for runtime
		m_Registry.view<ParticleSystemComponent>().each([dt](auto entity, auto& psc)
		{
			psc.UpdateParticleSystem(dt);
		});
	}

	void MainEditorScene::OnSceneRender()
	{
		auto spriteGroup = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);

		// Sort entities for rendering based on z position or creation order (if z positions are equivalent)
		spriteGroup.sort([&](const entt::entity lhs, const entt::entity rhs)
		{
			const auto& ltc = spriteGroup.get<TransformComponent>(lhs);
			const auto& rtc = spriteGroup.get<TransformComponent>(rhs);
			if (ltc.Translation.z == rtc.Translation.z)
			{
				return m_Registry.get<CoreComponent>(lhs).CreationId < m_Registry.get<CoreComponent>(rhs).CreationId;
			}
			return ltc.Translation.z < rtc.Translation.z;
		});

		for (auto entity : spriteGroup)
		{
			auto [transformComp, spriteComp] = spriteGroup.get<TransformComponent, SpriteRendererComponent>(entity);
			if (spriteComp.Texture)
			{
				Renderer2D::DrawRotatedQuad(transformComp.GetTransform(), spriteComp.Texture, spriteComp.TextureTiling, { 0.0f, 0.0f }, spriteComp.TintColor);
			}
			else
			{
				Renderer2D::DrawRotatedQuad(transformComp.GetTransform(), spriteComp.TintColor);
			}
		}

		// Render particle system for runtime
		m_Registry.view<ParticleSystemComponent>().each([](auto entity, auto& psc)
		{
			psc.RenderParticleSystem();
		});
	}

	void MainEditorScene::OnDeserialized()
	{
		m_Registry.view<ParticleSystemComponent>().each([](auto entity, auto& psc)
		{
			psc.CreateParticleSystem({});
		});
	}

	void MainEditorScene::OnClenup()
	{
		Scene::OnClenup();

		// Clear particle system reference for runtime on scene destruction
		m_Registry.view<ParticleSystemComponent>().each([](auto entity, auto& psc)
		{
			psc.RemoveParticleSystemInstance();
		});
	}

}

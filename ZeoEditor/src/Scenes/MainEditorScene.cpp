#include "Scenes/MainEditorScene.h"

#include "Engine/GameFramework/Components.h"
#include "Engine/Renderer/Renderer2D.h"

namespace ZeoEngine {

	MainEditorScene::~MainEditorScene()
	{
		OnClenup();
	}

	void MainEditorScene::OnUpdate(DeltaTime dt)
	{
		// TODO:
		OnUpdateEditor(dt);
	}

	void MainEditorScene::OnRender(const EditorCamera& camera)
	{
		// TODO:
		OnRenderEditor(camera);
	}

	void MainEditorScene::OnEvent(Event& e)
	{
		// TODO:
		m_Registry.view<NativeScriptComponent>().each([&e](auto entity, auto& nsc)
		{
			if (nsc.Instance)
			{
				nsc.Instance->OnEvent(e);
			}
		});
	}

	void MainEditorScene::OnUpdateEditor(DeltaTime dt)
	{
		UpdateParticleSystem(dt);
	}

	void MainEditorScene::OnUpdateRuntime(DeltaTime dt)
	{
		// Update scripts
		m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc)
		{
			// TODO: Move to Scene::OnBeginPlay
			if (!nsc.Instance)
			{
				nsc.Instance = nsc.InstantiateScript();
				nsc.Instance->m_Entity = Entity{ entity, this };
				nsc.Instance->OnCreate();
			}

			nsc.Instance->OnUpdate(dt);
		});

		UpdateParticleSystem(dt);
	}

	void MainEditorScene::UpdateParticleSystem(DeltaTime dt)
	{
		m_Registry.view<ParticleSystemComponent>().each([dt](auto entity, auto& psc)
		{
			psc.UpdateParticleSystem(dt);
		});
	}

	void MainEditorScene::OnRenderEditor(const EditorCamera& camera)
	{
		Renderer2D::BeginScene(camera);
		{
			RenderPrimitives();
		}
		Renderer2D::EndScene();
	}

	void MainEditorScene::OnRenderRuntime()
	{
		// Render 2D
		Camera* mainCamera = nullptr;
		glm::mat4 cameraTransform;
		{
			auto cameraView = m_Registry.view<TransformComponent, CameraComponent>();
			for (auto entity : cameraView)
			{
				auto [transformComp, cameraComp] = cameraView.get<TransformComponent, CameraComponent>(entity);
				if (cameraComp.bIsPrimary)
				{
					mainCamera = &cameraComp.Camera;
					cameraTransform = transformComp.GetTransform();
					break;
				}
			}
		}
		if (mainCamera)
		{
			Renderer2D::BeginScene(*mainCamera, cameraTransform);
			{
				RenderPrimitives();
			}
			Renderer2D::EndScene();
		}
	}

	void MainEditorScene::RenderPrimitives()
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
				Renderer2D::DrawRotatedQuad(transformComp.GetTransform(), spriteComp.Texture, spriteComp.TextureTiling, { 0.0f, 0.0f }, spriteComp.TintColor, static_cast<uint32_t>(entity));
			}
			else
			{
				Renderer2D::DrawRotatedQuad(transformComp.GetTransform(), spriteComp.TintColor, static_cast<uint32_t>(entity));
			}
		}

		// Render particle system
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
			psc.RemoveCurrentParticleSystemInstance();
		});
	}

}

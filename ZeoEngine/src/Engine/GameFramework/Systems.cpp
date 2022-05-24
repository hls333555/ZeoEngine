#include "ZEpch.h"
#include "Engine/GameFramework/Systems.h"

#include <box2d/b2_world.h>
#include <box2d/b2_body.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_circle_shape.h>

#include "Engine/GameFramework/Components.h"
#include "Engine/Renderer/Renderer2D.h"
#include "Engine/Renderer/SceneRenderer.h"
#include "Engine/GameFramework/ScriptableEntity.h"
#include "Engine/Utils/DebugDrawUtils.h"

namespace ZeoEngine {

	ISystem::ISystem(const Ref<Scene>& scene)
		: m_Scene(scene)
		, m_SceneRaw(scene.get())
	{
	}

	SystemBase::SystemBase(const Ref<Scene>& scene)
		: ISystem(scene)
	{
		BindUpdateFuncToEditor();
	}

	void SystemBase::OnUpdate(DeltaTime dt) const
	{
		m_UpdateFuncDel(dt);
	}

	void SystemBase::BindUpdateFuncToEditor()
	{
		m_UpdateFuncDel.connect<&SystemBase::OnUpdateEditor>(this);
	}

	void SystemBase::BindUpdateFuncToRuntime()
	{
		m_UpdateFuncDel.connect<&SystemBase::OnUpdateRuntime>(this);
	}

	RenderSystemBase::RenderSystemBase(const Ref<SceneRenderer>& sceneRenderer)
		: ISystem(nullptr)
		, m_SceneRenderer(sceneRenderer)
	{
	}

	std::pair<Camera*, glm::mat4> RenderSystemBase::GetActiveCamera()
	{
		Camera* camera = nullptr;
		glm::mat4 transform;
		ForEachComponentView<TransformComponent, CameraComponent>([&camera, &transform](auto entity, auto& transformComp, auto& cameraComp)
		{
			if (cameraComp.bIsPrimary)
			{
				camera = &cameraComp.Camera;
				transform = transformComp.GetTransform();
				return;
			}
		});
		return std::make_pair(camera, transform);
	}

	void RenderSystem::OnRenderEditor()
	{
		RenderLights(true);
		RenderMeshes();

		// Render billboards
		ForEachComponentView<TransformComponent, BillboardComponent>([this](auto e, auto& transformComp, auto& billboardComp)
		{
			if (billboardComp.TextureAsset)
			{
				const Entity entity = { e, GetScene() };
				const glm::vec4 tintColor = entity.HasComponent<LightComponent>() ? entity.GetComponent<LightComponent>().GetColor() : glm::vec4(1.0f);
				m_SceneRenderer->DrawBillboard(transformComp.Translation, billboardComp.Size, billboardComp.TextureAsset, { 1.0f, 1.0f }, { 0.0f, 0.0f }, tintColor, static_cast<int32_t>(e));
			}
		});

		// Render camera frustums
		ForEachComponentView<TransformComponent, CameraComponent>([this](auto entity, auto& transformComp, auto& cameraComp)
		{
			static const glm::vec3 visualizeColor = { 0.5f, 0.5f, 0.5f };
			// TODO: Replace this with FrameBuffer texture
			// Draw frustum visualizer when selected
			if (GetSceneRaw()->GetSelectedEntity() == entity)
			{
				glm::mat4 invMatix = transformComp.GetTransform() * glm::inverse(cameraComp.Camera.GetProjection());
				DebugDrawUtils::DrawFrustum(GetScene(), invMatix, visualizeColor);
			}
		});
	}

	void RenderSystem::OnRenderRuntime()
	{
		RenderLights(false);
		RenderMeshes();
	}

	void RenderSystem::RenderLights(bool bIsEditor)
	{
		ForEachComponentView<TransformComponent, LightComponent, BillboardComponent>([this, bIsEditor](auto entity, auto& transformComp, auto& lightComp, auto& billboardComp)
		{
			static const glm::vec3 visualizeColor = { 0.5f, 0.5f, 0.5f };
			switch (lightComp.Type)
			{
				case LightComponent::LightType::DirectionalLight:
				{
					const auto& directionalLight = lightComp.GetLight<DirectionalLight>();
					m_SceneRenderer->SetupDirectionalLight(transformComp.Rotation, directionalLight);

					// Draw arrow visualizer when selected
					if (bIsEditor && GetSceneRaw()->GetSelectedEntity() == entity)
					{
						const auto forward = glm::rotate(glm::quat(transformComp.Rotation), { 0.0f, 0.0f, -1.0f });
						const auto endPosition = transformComp.Translation + glm::normalize(forward);
						DebugDrawUtils::DrawArrow(GetScene(), transformComp.Translation, endPosition, visualizeColor, 0.25f);
					}
					break;
				}
				case LightComponent::LightType::PointLight:
				{
					const auto& pointLight = lightComp.GetLight<PointLight>();
					m_SceneRenderer->AddPointLight(transformComp.Translation, pointLight);

					// Draw sphere visualizer when selected
					if (bIsEditor && GetSceneRaw()->GetSelectedEntity() == entity)
					{
						DebugDrawUtils::DrawSphereBounds(GetScene(), transformComp.Translation, visualizeColor, pointLight->GetRange());
					}
					break;
				}
				case LightComponent::LightType::SpotLight:
				{
					const auto& spotLight = lightComp.GetLight<SpotLight>();
					m_SceneRenderer->AddSpotLight(transformComp.Translation, transformComp.Rotation, spotLight);

					// Draw cone visualizer when selected
					if (bIsEditor && GetSceneRaw()->GetSelectedEntity() == entity)
					{
						const auto direction = spotLight->CalculateDirection(transformComp.Rotation) * spotLight->GetRange();
						const auto radius = tan(spotLight->GetCutoff()) * spotLight->GetRange();
						DebugDrawUtils::DrawCone(GetScene(), transformComp.Translation, direction, visualizeColor, radius, 0.0f);
					}
					break;
				}
				default:
					break;
			}
		});
	}

	void RenderSystem::RenderMeshes()
	{
		ForEachComponentGroup<TransformComponent>(entt::get<MeshRendererComponent>, [this](auto entity, auto& transformComp, auto& meshComp)
		{
			m_SceneRenderer->DrawMesh(transformComp.GetTransform(), meshComp.Instance, static_cast<int32_t>(entity));
		});
	}

	static void RemoveParticleSystemInstance(ParticleSystemComponent& particleComp)
	{
		if (particleComp.ParticleTemplateAsset)
		{
			particleComp.ParticleTemplateAsset->RemoveParticleSystemInstance(particleComp.Instance);
		}
	}

	void RenderSystem2D::OnRenderEditor()
	{
		OnRenderImpl();
	}

	void RenderSystem2D::OnRenderRuntime()
	{
		OnRenderImpl();
	}

	void RenderSystem2D::OnRenderImpl()
	{
		// Render sprites
		ForEachComponentGroup<TransformComponent>(entt::get<SpriteRendererComponent>, [](auto entity, auto& transformComp, auto& spriteComp)
		{
			Renderer2D::DrawSprite(transformComp.GetTransform(), spriteComp, static_cast<int32_t>(entity));
		},
		[](std::tuple<TransformComponent&, SpriteRendererComponent&> lhs, std::tuple<TransformComponent&, SpriteRendererComponent&> rhs)
		{
			const auto& lSpriteComp = std::get<1>(lhs);
			const auto& rSpriteComp = std::get<1>(rhs);
			return lSpriteComp.SortingOrder < rSpriteComp.SortingOrder;
		});

		// Render circles
		ForEachComponentView<TransformComponent, CircleRendererComponent>([](auto entity, auto& transformComp, auto& circleComp)
		{
			Renderer2D::DrawCircle(transformComp.GetTransform(), circleComp.Color, circleComp.Thickness, circleComp.Fade, static_cast<int32_t>(entity));
		});

		// Render particle systems
		ForEachComponentView<ParticleSystemComponent>([](auto entity, auto& particleComp)
		{
			if (particleComp.Instance)
			{
				particleComp.Instance->OnRender();
			}
		});
	}

	void ParticlePreviewRenderSystem::OnRenderEditor()
	{
		ForEachComponentView<ParticleSystemPreviewComponent>([](auto entity, auto& particlePreviewComp)
		{
			particlePreviewComp.Instance->OnRender();
		});
	}

	void MaterialPreviewRenderSystem::OnRenderEditor()
	{
		ForEachComponentView<TransformComponent, LightComponent>([this](auto entity, auto& transformComp, auto& lightComp)
		{
			m_SceneRenderer->SetupDirectionalLight(transformComp.Rotation, lightComp.GetLight<DirectionalLight>());
		});
		ForEachComponentGroup<TransformComponent>(entt::get<MeshRendererComponent, MaterialPreviewComponent>, [this](auto entity, auto& transformComp, auto& meshComp, auto& materialPreviewComp)
		{
			m_SceneRenderer->DrawMesh(transformComp.GetTransform(), meshComp.Instance);
		});
	}

	void ParticleUpdateSystem::OnUpdateEditor(DeltaTime dt)
	{
		OnUpdateImpl(dt);
	}

	void ParticleUpdateSystem::OnUpdateRuntime(DeltaTime dt)
	{
		OnUpdateImpl(dt);
	}

	void ParticleUpdateSystem::OnUpdateImpl(DeltaTime dt)
	{
		ForEachComponentView<ParticleSystemComponent>([dt](auto entity, auto& particleComp)
		{
			if (particleComp.Instance)
			{
				particleComp.Instance->OnUpdate(dt);
			}
		});
	}

	void ParticleUpdateSystem::OnDestroy()
	{
		ForEachComponentView<ParticleSystemComponent>([](auto entity, auto& particleComp)
		{
			RemoveParticleSystemInstance(particleComp);
		});
	}

	void ParticlePreviewUpdateSystem::OnUpdateEditor(DeltaTime dt)
	{
		OnUpdateImpl(dt);
	}

	void ParticlePreviewUpdateSystem::OnUpdateRuntime(DeltaTime dt)
	{
		OnUpdateImpl(dt);
	}

	void ParticlePreviewUpdateSystem::OnUpdateImpl(DeltaTime dt)
	{
		ForEachComponentView<ParticleSystemPreviewComponent>([dt](auto entity, auto& particlePreviewComp)
		{
			if (particlePreviewComp.Instance)
			{
				particlePreviewComp.Instance->OnUpdate(dt);
			}
		});
	}

	void ParticlePreviewUpdateSystem::OnDestroy()
	{
		ForEachComponentView<ParticleSystemPreviewComponent>([](auto entity, auto& particlePreviewComp)
		{
			RemoveParticleSystemInstance(particlePreviewComp);
		});
	}

	void NativeScriptSystem::OnUpdateRuntime(DeltaTime dt)
	{
		ForEachComponentView<NativeScriptComponent>([=](auto entity, auto& nativeScriptComp)
		{
			// TODO: Move to OnBeginPlay
			if (!nativeScriptComp.Instance)
			{
				nativeScriptComp.Instance = nativeScriptComp.InstantiateScript();
				nativeScriptComp.Instance->m_Entity = Entity{ entity, GetScene() };
				nativeScriptComp.Instance->OnCreate();
			}

			nativeScriptComp.Instance->OnUpdate(dt);
		});
	}

	void NativeScriptSystem::OnEvent(Event& e)
	{
		ForEachComponentView<NativeScriptComponent>([&e](auto entity, auto& nativeScriptComp)
		{
			if (nativeScriptComp.Instance)
			{
				nativeScriptComp.Instance->OnEvent(e);
			}
		});
	}

	void PhysicsSystem::OnUpdateRuntime(DeltaTime dt)
	{

	}


	void PhysicsSystem::OnRuntimeStart()
	{

	}


	void PhysicsSystem::OnRuntimeStop()
	{

	}

	void PhysicsSystem2D::OnUpdateRuntime(DeltaTime dt)
	{
		const int32_t velocityIterations = 6;
		const int32_t positionIterations = 2;
		m_PhysicsWorld->Step(dt, velocityIterations, positionIterations);

		ForEachComponentView<Rigidbody2DComponent>([this](auto e, auto& rb2dComp)
		{
			const Entity entity = { e, GetScene() };
			auto& transformComp = entity.GetComponent<TransformComponent>();

			// Retrieve transfrom from Box2D
			const b2Body* body = static_cast<b2Body*>(rb2dComp.RuntimeBody);
			const auto& position = body->GetPosition();
			transformComp.Translation.x = position.x;
			transformComp.Translation.y = position.y;
			transformComp.Rotation.z = body->GetAngle();
		});
	}

	static b2BodyType Rigidbody2DTypeToBox2DBody(Rigidbody2DComponent::BodyType bodyType)
	{
		switch (bodyType)
		{
			case Rigidbody2DComponent::BodyType::Static:	return b2_staticBody;
			case Rigidbody2DComponent::BodyType::Dynamic:	return b2_dynamicBody;
			case Rigidbody2DComponent::BodyType::Kinematic:	return b2_kinematicBody;
		}

		ZE_CORE_ASSERT(false, "Unknown body type!");
		return b2_staticBody;
	}

	void PhysicsSystem2D::OnRuntimeStart()
	{
		const b2Vec2 gravity = { 0.0f, -9.8f };
		m_PhysicsWorld = new b2World(gravity);
		ForEachComponentView<Rigidbody2DComponent>([this](auto e, auto& rb2dComp)
		{
			const Entity entity = { e, GetScene() };
			const auto& transformComp = entity.GetComponent<TransformComponent>();

			b2BodyDef bodyDef;
			bodyDef.type = Rigidbody2DTypeToBox2DBody(rb2dComp.Type);
			bodyDef.position.Set(transformComp.Translation.x, transformComp.Translation.y);
			bodyDef.angle = transformComp.Rotation.z;

			b2Body* body = m_PhysicsWorld->CreateBody(&bodyDef);
			body->SetFixedRotation(rb2dComp.bFixedRotation);
			rb2dComp.RuntimeBody = body;

			if (entity.HasComponent<BoxCollider2DComponent>())
			{
				const auto& bc2dComp = entity.GetComponent<BoxCollider2DComponent>();

				b2PolygonShape boxShape;
				boxShape.SetAsBox(bc2dComp.Size.x * transformComp.Scale.x, bc2dComp.Size.y * transformComp.Scale.y);

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &boxShape;
				fixtureDef.density = bc2dComp.Density;
				fixtureDef.friction = bc2dComp.Friction;
				fixtureDef.restitution = bc2dComp.Restitution;
				fixtureDef.restitutionThreshold = bc2dComp.RestitutionThreshold;
				body->CreateFixture(&fixtureDef);
			}

			if (entity.HasComponent<CircleCollider2DComponent>())
			{
				const auto& cc2dComp = entity.GetComponent<CircleCollider2DComponent>();

				b2CircleShape circleShape;
				circleShape.m_p.Set(cc2dComp.Offset.x, cc2dComp.Offset.y);
				circleShape.m_radius = cc2dComp.Radius;

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &circleShape;
				fixtureDef.density = cc2dComp.Density;
				fixtureDef.friction = cc2dComp.Friction;
				fixtureDef.restitution = cc2dComp.Restitution;
				fixtureDef.restitutionThreshold = cc2dComp.RestitutionThreshold;
				body->CreateFixture(&fixtureDef);
			}
		});
	}

	void PhysicsSystem2D::OnRuntimeStop()
	{
		delete m_PhysicsWorld;
		m_PhysicsWorld = nullptr;
	}

}

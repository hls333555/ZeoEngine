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
#include "Engine/Scripting/ScriptEngine.h"
#include "Engine/Utils/DebugDrawUtils.h"
#include "Engine/GameFramework/World.h"

namespace ZeoEngine {

	ISystem::ISystem(const Ref<WorldBase>& world)
		: m_World(world)
	{
	}

	SystemBase::SystemBase(const Ref<WorldBase>& world)
		: ISystem(world)
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

	RenderSystemBase::RenderSystemBase(const Ref<WorldBase>& world)
		: ISystem(world)
		, m_SceneRenderer(world->GetSceneRenderer())
	{
	}

	std::pair<SceneCamera*, Mat4> RenderSystemBase::GetActiveSceneCamera() const
	{
		SceneCamera* camera = nullptr;
		Mat4 transform;
		GetScene()->ForEachComponentView<TransformComponent, CameraComponent>([&camera, &transform](auto entity, auto& transformComp, auto& cameraComp)
		{
			if (cameraComp.bIsPrimary)
			{
				camera = &cameraComp.Camera;
				transform = transformComp.GetTransform();
			}
		});
		return std::make_pair(camera, transform);
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
		GetScene()->ForEachComponentView<ParticleSystemComponent>([dt](auto entity, auto& particleComp)
		{
			if (particleComp.Instance)
			{
				particleComp.Instance->OnUpdate(dt);
			}
		});
	}

	void ParticlePreviewUpdateSystem::OnUpdateImpl(DeltaTime dt)
	{
		GetScene()->ForEachComponentView<ParticleSystemPreviewComponent>([dt](auto entity, auto& particlePreviewComp)
		{
			if (particlePreviewComp.Instance)
			{
				particlePreviewComp.Instance->OnUpdate(dt);
			}
		});
	}

	void ScriptSystem::OnUpdateRuntime(DeltaTime dt)
	{
		GetScene()->ForEachComponentView<ScriptComponent>([this, dt](auto e, auto& scriptComp)
		{
			const Entity entity = { e, GetScene() };
			ScriptEngine::OnUpdateEntity(entity, dt);
		});
	}

	void ScriptSystem::OnRuntimeStart()
	{
		ScriptEngine::OnRuntimeStart(GetScene());
		GetScene()->ForEachComponentView<ScriptComponent>([this](auto e, auto& scriptComp)
		{
			const Entity entity = { e, GetScene() };
			ScriptEngine::OnCreateEntity(entity);
		});
	}

	void ScriptSystem::OnRuntimeStop()
	{
		ScriptEngine::OnRuntimeStop();
	}

	void NativeScriptSystem::OnUpdateRuntime(DeltaTime dt)
	{
		GetScene()->ForEachComponentView<NativeScriptComponent>([=](auto entity, auto& nativeScriptComp)
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
		GetScene()->ForEachComponentView<NativeScriptComponent>([&e](auto entity, auto& nativeScriptComp)
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
		const I32 velocityIterations = 6;
		const I32 positionIterations = 2;
		m_PhysicsWorld->Step(dt, velocityIterations, positionIterations);

		GetScene()->ForEachComponentView<Rigidbody2DComponent>([this](auto e, auto& rb2dComp)
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
		GetScene()->ForEachComponentView<Rigidbody2DComponent>([this](auto e, auto& rb2dComp)
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

	static Vec3 debugDrawColor = { 0.7f, 0.7f, 0.7f };

	void RenderSystem::OnRenderEditor()
	{
		RenderLights(true);
		RenderMeshes();

		// Render billboards
		GetScene()->ForEachComponentView<TransformComponent, BillboardComponent>([this](auto e, auto& transformComp, auto& billboardComp)
		{
			if (billboardComp.TextureAsset)
			{
				const Entity entity = { e, GetScene() };
				const Vec4 tintColor = entity.HasComponent<LightComponent>() ? entity.GetComponent<LightComponent>().GetColor() : Vec4(1.0f);
				GetSceneRenderer()->DrawBillboard(transformComp.Translation, billboardComp.Size, billboardComp.TextureAsset, { 1.0f, 1.0f }, { 0.0f, 0.0f }, tintColor, static_cast<I32>(e));
			}
		});

		// Render camera frustums
		GetScene()->ForEachComponentView<TransformComponent, CameraComponent>([this](auto entity, auto& transformComp, auto& cameraComp)
		{
			// TODO: Replace this with FrameBuffer texture
			// Draw frustum visualizer when selected
			if (GetWorld()->GetContextEntity() == entity)
			{
				const Mat4 invMatrix = transformComp.GetTransform() * glm::inverse(cameraComp.Camera.GetProjection());
				DebugDrawUtils::DrawFrustum(GetScene(), invMatrix, debugDrawColor);
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
		GetScene()->ForEachComponentView<TransformComponent, LightComponent, BillboardComponent>([this, bIsEditor](auto entity, auto& transformComp, auto& lightComp, auto& billboardComp)
		{
			switch (lightComp.Type)
			{
				case LightComponent::LightType::DirectionalLight:
				{
					const auto& directionalLight = lightComp.GetLight<DirectionalLight>();
					GetSceneRenderer()->SetupDirectionalLight(transformComp.Rotation, directionalLight);

					// Draw arrow visualizer when selected
					if (bIsEditor && GetWorld()->GetContextEntity() == entity)
					{
						const auto forward = glm::rotate(glm::quat(transformComp.Rotation), { 0.0f, 0.0f, -1.0f });
						const auto endPosition = transformComp.Translation + glm::normalize(forward);
						DebugDrawUtils::DrawArrow(GetScene(), transformComp.Translation, endPosition, debugDrawColor, 0.25f);
					}
					break;
				}
				case LightComponent::LightType::PointLight:
				{
					const auto& pointLight = lightComp.GetLight<PointLight>();
					GetSceneRenderer()->AddPointLight(transformComp.Translation, pointLight);

					// Draw sphere visualizer when selected
					if (bIsEditor && GetWorld()->GetContextEntity() == entity)
					{
						DebugDrawUtils::DrawSphereBounds(GetScene(), transformComp.Translation, debugDrawColor, pointLight->GetRange());
					}
					break;
				}
				case LightComponent::LightType::SpotLight:
				{
					const auto& spotLight = lightComp.GetLight<SpotLight>();
					GetSceneRenderer()->AddSpotLight(transformComp.Translation, transformComp.Rotation, spotLight);

					// Draw cone visualizer when selected
					if (bIsEditor && GetWorld()->GetContextEntity() == entity)
					{
						const auto direction = spotLight->CalculateDirection(transformComp.Rotation) * spotLight->GetRange();
						const auto radius = tan(spotLight->GetCutoff()) * spotLight->GetRange();
						DebugDrawUtils::DrawCone(GetScene(), transformComp.Translation, direction, debugDrawColor, radius, 0.0f);
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
		GetScene()->ForEachComponentGroup<TransformComponent>(entt::get<MeshRendererComponent/*, BoundsComponent*/>, [this](auto entity, auto& transformComp, auto& meshComp/*, auto& boundsComp*/)
		{
			GetSceneRenderer()->DrawMesh(transformComp.GetTransform(), meshComp.Instance, static_cast<I32>(entity));
			//DebugDrawUtils::DrawSphereBounds(GetScene(), boundsComp.Bounds.Origin, debugDrawColor, boundsComp.Bounds.SphereRadius);
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
		GetScene()->ForEachComponentGroup<TransformComponent>(entt::get<SpriteRendererComponent>, [](auto entity, auto& transformComp, auto& spriteComp)
		{
			Renderer2D::DrawSprite(transformComp.GetTransform(), spriteComp, static_cast<I32>(entity));
		},
		[](std::tuple<TransformComponent&, SpriteRendererComponent&> lhs, std::tuple<TransformComponent&, SpriteRendererComponent&> rhs)
		{
			const auto& lSpriteComp = std::get<1>(lhs);
			const auto& rSpriteComp = std::get<1>(rhs);
			return lSpriteComp.SortingOrder < rSpriteComp.SortingOrder;
		});

		// Render circles
		GetScene()->ForEachComponentView<TransformComponent, CircleRendererComponent>([](auto entity, auto& transformComp, auto& circleComp)
		{
			Renderer2D::DrawCircle(transformComp.GetTransform(), circleComp.Color, circleComp.Thickness, circleComp.Fade, static_cast<I32>(entity));
		});

		// Render particle systems
		GetScene()->ForEachComponentView<ParticleSystemComponent>([](auto entity, auto& particleComp)
		{
			if (particleComp.Instance)
			{
				particleComp.Instance->OnRender();
			}
		});
	}

	void ParticlePreviewRenderSystem::OnRenderEditor()
	{
		GetScene()->ForEachComponentView<ParticleSystemPreviewComponent>([](auto entity, auto& particlePreviewComp)
		{
			particlePreviewComp.Instance->OnRender();
		});
	}

	void MeshPreviewRenderSystem::OnRenderEditor()
	{
		GetScene()->ForEachComponentView<TransformComponent, LightComponent>([this](auto entity, auto& transformComp, auto& lightComp)
		{
			GetSceneRenderer()->SetupDirectionalLight(transformComp.Rotation, lightComp.GetLight<DirectionalLight>());
		});
		GetScene()->ForEachComponentGroup<TransformComponent>(entt::get<MeshPreviewComponent/*, BoundsComponent*/>, [this](auto entity, auto& transformComp, auto& meshComp/*, auto& boundsComp*/)
		{
			GetSceneRenderer()->DrawMesh(transformComp.GetTransform(), meshComp.Instance);
			//DebugDrawUtils::DrawSphereBounds(GetScene(), boundsComp.Bounds.Origin, debugDrawColor, boundsComp.Bounds.SphereRadius);
		});
	}

	void MaterialPreviewRenderSystem::OnRenderEditor()
	{
		GetScene()->ForEachComponentView<TransformComponent, LightComponent>([this](auto entity, auto& transformComp, auto& lightComp)
		{
			GetSceneRenderer()->SetupDirectionalLight(transformComp.Rotation, lightComp.GetLight<DirectionalLight>());
		});
		GetScene()->ForEachComponentGroup<TransformComponent>(entt::get<MeshRendererComponent>, [this](auto entity, auto& transformComp, auto& meshComp)
		{
			GetSceneRenderer()->DrawMesh(transformComp.GetTransform(), meshComp.Instance);
		});
	}

}

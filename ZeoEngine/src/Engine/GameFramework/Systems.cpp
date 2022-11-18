#include "ZEpch.h"
#include "Engine/GameFramework/Systems.h"

#include <box2d/b2_world.h>
#include <box2d/b2_body.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_circle_shape.h>

#include "Engine/Core/Console.h"
#include "Engine/Core/ConsoleVariables.h"
#include "Engine/GameFramework/Components.h"
#include "Engine/Renderer/SceneRenderer.h"
#include "Engine/Scripting/ScriptEngine.h"
#include "Engine/GameFramework/World.h"
#include "Engine/Physics/PhysicsEngine.h"
#include "Engine/Physics/PhysXScene.h"
#include "Engine/Renderer/RenderPass.h"

namespace ZeoEngine {

	ISystem::ISystem(WorldBase* world)
		: m_World(world)
	{
	}

	// TODO: Move to class static functions
	static void OnCameraComponentAdded(Scene& scene, entt::entity e)
	{
		Entity entity{ e, scene.shared_from_this() };
		auto& billboardComp = entity.AddComponent<BillboardComponent>();
		billboardComp.TextureAsset = AssetLibrary::LoadAsset<Texture2D>("assets/textures/icons/Camera.png.zasset");
	}

	static void OnCameraComponentDestroy(Scene& scene, entt::entity e)
	{
		Entity entity{ e, scene.shared_from_this() };
		entity.RemoveComponentIfExist<BillboardComponent>();
	}

	static void OnMeshRendererComponentAdded(Scene& scene, entt::entity e)
	{
		Entity entity{ e, scene.shared_from_this() };
		const auto& meshComp = entity.GetComponent<MeshRendererComponent>();
		// Invoke update callback for ctor with MeshAsset provided but not for copy ctor
		if (meshComp.MeshAsset && !meshComp.Instance)
		{
			entity.PatchComponentSingleField<MeshRendererComponent>("MeshAsset"_hs);
		}
		auto& boundsComp = entity.GetComponent<BoundsComponent>();
		boundsComp.BoundsCalculationFuncs[entt::type_hash<MeshRendererComponent>::value()] = [](Entity entity)
		{
			const auto& transformComp = entity.GetComponent<TransformComponent>();
			const auto& meshComp = entity.GetComponent<MeshRendererComponent>();
			const auto mesh = AssetLibrary::LoadAsset<Mesh>(meshComp.MeshAsset);
			return mesh ? mesh->GetBounds().TransformBy(transformComp.GetTransform()) : BoxSphereBounds{};
		};
	}

	static void OnMeshRendererComponentFieldChanged(Scene& scene, entt::entity e)
	{
		const Entity entity{ e, scene.shared_from_this() };
		auto& meshComp = entity.GetComponent<MeshRendererComponent>();
		const auto& changeComp = entity.GetComponent<FieldChangeComponent>();
		if (changeComp.FieldID == "MeshAsset"_hs)
		{
			auto& meshInstance = meshComp.Instance;
			const auto mesh = AssetLibrary::LoadAsset<Mesh>(meshComp.MeshAsset);
			if (mesh)
			{
				// Copy default materials
				// For deserialization phase, materials will be deserialized and overwritten later
				meshComp.MaterialAssets = mesh->GetDefaultMaterialAssets();
				meshInstance = mesh->CreateInstance(scene);
			}
			else
			{
				meshInstance = nullptr;
				meshComp.MaterialAssets.clear();
			}
			// Update bounds immediately so that focusing after loading should work properly
			entity.UpdateBounds();
		}
	}

	static void OnMeshRendererComponentDestroy(Scene& scene, entt::entity e)
	{
		Entity entity{ e, scene.shared_from_this() };
		if (entity.HasComponent<BoundsComponent>())
		{
			entity.PatchComponent<BoundsComponent>([](BoundsComponent& boundsComp)
			{
				boundsComp.BoundsCalculationFuncs.erase(entt::type_hash<MeshRendererComponent>::value());
			});
		}
	}

	static void OnMeshDetailComponentAdded(Scene& scene, entt::entity e)
	{
		const Entity entity{ e, scene.shared_from_this() };
		auto& boundsComp = entity.GetComponent<BoundsComponent>();
		boundsComp.BoundsCalculationFuncs[entt::type_hash<MeshDetailComponent>::value()] = [](Entity entity)
		{
			const auto& transformComp = entity.GetComponent<TransformComponent>();
			const auto& meshComp = entity.GetComponent<MeshDetailComponent>();
			const auto& mesh = meshComp.LoadedMesh;
			return mesh ? mesh->GetBounds().TransformBy(transformComp.GetTransform()) : BoxSphereBounds{};
		};
	}

	static void OnMeshDetailComponentFieldChanged(Scene& scene, entt::entity e)
	{
		const Entity entity{ e, scene.shared_from_this() };
		auto& meshComp = entity.GetComponent<MeshDetailComponent>();
		const auto& changeComp = entity.GetComponent<FieldChangeComponent>();
		if (changeComp.FieldID == "LoadedMesh"_hs)
		{
			meshComp.Instance = meshComp.LoadedMesh->CreateInstance(scene);
			// Update bounds immediately so that focusing after loading should work properly
			entity.UpdateBounds();
		}
	}

	static void OnMeshDetailComponentDestroy(Scene& scene, entt::entity e)
	{
		Entity entity{ e, scene.shared_from_this() };
		if (entity.HasComponent<BoundsComponent>())
		{
			entity.PatchComponent<BoundsComponent>([](BoundsComponent& boundsComp)
			{
				boundsComp.BoundsCalculationFuncs.erase(entt::type_hash<MeshDetailComponent>::value());
			});
		}
	}

	static void OnMaterialDetailComponentFieldChanged(Scene& scene, entt::entity e)
	{
		const Entity entity{ e, scene.shared_from_this() };
		const auto& materialComp = entity.GetComponent<MaterialDetailComponent>();
		const auto& changeComp = entity.GetComponent<FieldChangeComponent>();
		if (changeComp.FieldID == "ShaderAsset"_hs)
		{
			materialComp.LoadedMaterial->NotifyShaderAssetChange();
		}
	}

	static void OnTextureDetailComponentFieldChanged(Scene& scene, entt::entity e)
	{
		const Entity entity{ e, scene.shared_from_this() };
		const auto& textureComp = entity.GetComponent<TextureDetailComponent>();
		const auto& changeComp = entity.GetComponent<FieldChangeComponent>();
		if (changeComp.FieldID == "SRGB"_hs || changeComp.FieldID == "GenerateMipmaps"_hs)
		{
			textureComp.LoadedTexture->Invalidate();
		}
		else if (changeComp.FieldID == "SamplerType"_hs)
		{
			textureComp.LoadedTexture->ChangeSampler(textureComp.SamplerType);
		}
	}

	static void OnScriptComponentFieldChanged(Scene& scene, entt::entity e)
	{
		const Entity entity{ e, scene.shared_from_this() };
		const auto& changeComp = entity.GetComponent<FieldChangeComponent>();
		if (changeComp.FieldID == "ClassName"_hs)
		{
			ScriptEngine::OnScriptClassChanged(entity);
		}
	}

	static void OnScriptComponentDestroy(Scene& scene, entt::entity e)
	{
		const Entity entity{ e, scene.shared_from_this() };
		ScriptEngine::OnDestroyEntity(entity);
	}

	static void OnDirectionalLightComponentAdded(Scene& scene, entt::entity e)
	{
		Entity entity{ e, scene.shared_from_this() };
		auto& billboardComp = entity.AddComponent<BillboardComponent>();
		billboardComp.TextureAsset = AssetLibrary::LoadAsset<Texture2D>("assets/textures/icons/DirectionalLight.png.zasset");
		entity.PatchComponent<BoundsComponent>([](BoundsComponent boundsComp)
		{
			boundsComp.BoundsCalculationFuncs[entt::type_hash<DirectionalLightComponent>::value()] = [](Entity entity)
			{
				const auto& transformComp = entity.GetComponent<TransformComponent>();
				const Sphere sphere{ transformComp.Translation, 0.0f };
				return sphere;
			};
		});
	}

	static void OnDirectionalLightComponentFieldChanged(Scene& scene, entt::entity e)
	{
		const Entity entity{ e, scene.shared_from_this() };
		auto& lightComp = entity.GetComponent<DirectionalLightComponent>();
		const auto& changeComp = entity.GetComponent<FieldChangeComponent>();
		if (changeComp.FieldID == "ShadowType"_hs)
		{
			const auto* shadowPass = EngineUtils::GetRenderPassFromContext<ScreenSpaceShadowPass>(scene.GetContextShared(), "ScreenSpaceShadow");
			const auto shadowShader = shadowPass->GetShadowShader();
			shadowShader->SetShaderVariantByMacro("SHADOW_TYPE", static_cast<U32>(lightComp.ShadowType));
		}
	}

	static void OnDirectionalLightComponentDestroy(Scene& scene, entt::entity e)
	{
		Entity entity{ e, scene.shared_from_this() };
		entity.RemoveComponentIfExist<BillboardComponent>();
		if (entity.HasComponent<BoundsComponent>())
		{
			entity.PatchComponent<BoundsComponent>([](BoundsComponent& boundsComp)
			{
				boundsComp.BoundsCalculationFuncs.erase(entt::type_hash<DirectionalLightComponent>::value());
			});
		}
	}

	static void OnPointLightComponentAdded(Scene& scene, entt::entity e)
	{
		Entity entity{ e, scene.shared_from_this() };
		auto& billboardComp = entity.AddComponent<BillboardComponent>();
		billboardComp.TextureAsset = AssetLibrary::LoadAsset<Texture2D>("assets/textures/icons/PointLight.png.zasset");
		entity.PatchComponent<BoundsComponent>([](BoundsComponent& boundsComp)
		{
			boundsComp.BoundsCalculationFuncs[entt::type_hash<PointLightComponent>::value()] = [](Entity entity)
			{
				const auto& transformComp = entity.GetComponent<TransformComponent>();
				const auto& lightComp = entity.GetComponent<PointLightComponent>();
				const Sphere sphere{ transformComp.Translation, lightComp.Range };
				return sphere;
			};
		});		
	}

	static void OnPointLightComponentFieldChanged(Scene& scene, entt::entity e)
	{
		Entity entity{ e, scene.shared_from_this() };
		auto& lightComp = entity.GetComponent<PointLightComponent>();
		const auto& changeComp = entity.GetComponent<FieldChangeComponent>();
		if (changeComp.FieldID == "ShadowType"_hs)
		{
			// TODO:
		}
		else if (changeComp.FieldID == "Range"_hs)
		{
			entity.PatchComponent<BoundsComponent>();
		}
	}

	static void OnPointLightComponentDestroy(Scene& scene, entt::entity e)
	{
		Entity entity{ e, scene.shared_from_this() };
		entity.RemoveComponentIfExist<BillboardComponent>();
		if (entity.HasComponent<BoundsComponent>())
		{
			entity.PatchComponent<BoundsComponent>([](BoundsComponent& boundsComp)
			{
				boundsComp.BoundsCalculationFuncs.erase(entt::type_hash<PointLightComponent>::value());
			});
		}
	}

	static void OnSpotLightComponentAdded(Scene& scene, entt::entity e)
	{
		Entity entity{ e, scene.shared_from_this() };
		auto& billboardComp = entity.AddComponent<BillboardComponent>();
		billboardComp.TextureAsset = AssetLibrary::LoadAsset<Texture2D>("assets/textures/icons/SpotLight.png.zasset");
		entity.PatchComponent<BoundsComponent>([](BoundsComponent& boundsComp)
		{
			boundsComp.BoundsCalculationFuncs[entt::type_hash<SpotLightComponent>::value()] = [](Entity entity)
			{
			   const auto& transformComp = entity.GetComponent<TransformComponent>();
			   const auto& lightComp = entity.GetComponent<SpotLightComponent>();
			   const Sphere sphere{ transformComp.Translation, lightComp.Range * 0.5f };
			   return sphere;
			};
		});
	}

	static void OnSpotLightComponentFieldChanged(Scene& scene, entt::entity e)
	{
		Entity entity{ e, scene.shared_from_this() };
		auto& lightComp = entity.GetComponent<SpotLightComponent>();
		const auto& changeComp = entity.GetComponent<FieldChangeComponent>();
		if (changeComp.FieldID == "ShadowType"_hs)
		{
			// TODO:
		}
		else if (changeComp.FieldID == "Range"_hs)
		{
			entity.PatchComponent<BoundsComponent>();
		}
	}

	static void OnSpotLightComponentDestroy(Scene& scene, entt::entity e)
	{
		Entity entity{ e, scene.shared_from_this() };
		entity.RemoveComponentIfExist<BillboardComponent>();
		if (entity.HasComponent<BoundsComponent>())
		{
			entity.PatchComponent<BoundsComponent>([](BoundsComponent& boundsComp)
			{
				boundsComp.BoundsCalculationFuncs.erase(entt::type_hash<SpotLightComponent>::value());
			});
		}	
	}

	void LevelObserverSystem::OnBind()
	{
		BindOnComponentAdded<CameraComponent, &OnCameraComponentAdded>();
		BindOnComponentDestroy<CameraComponent, &OnCameraComponentDestroy>();

		BindOnComponentAdded<MeshRendererComponent, &OnMeshRendererComponentAdded>();
		BindOnComponentUpdated<MeshRendererComponent, &OnMeshRendererComponentFieldChanged>();
		BindOnComponentDestroy<MeshRendererComponent, &OnMeshRendererComponentDestroy>();

		BindOnComponentUpdated<ScriptComponent, &OnScriptComponentFieldChanged>();
		BindOnComponentDestroy<ScriptComponent, &OnScriptComponentDestroy>();

		BindOnComponentAdded<DirectionalLightComponent, &OnDirectionalLightComponentAdded>();
		BindOnComponentUpdated<DirectionalLightComponent, &OnDirectionalLightComponentFieldChanged>();
		BindOnComponentDestroy<DirectionalLightComponent, &OnDirectionalLightComponentDestroy>();

		BindOnComponentAdded<PointLightComponent, &OnPointLightComponentAdded>();
		BindOnComponentUpdated<PointLightComponent, &OnPointLightComponentFieldChanged>();
		BindOnComponentDestroy<PointLightComponent, &OnPointLightComponentDestroy>();

		BindOnComponentAdded<SpotLightComponent, &OnSpotLightComponentAdded>();
		BindOnComponentUpdated<SpotLightComponent, &OnSpotLightComponentFieldChanged>();
		BindOnComponentDestroy<SpotLightComponent, &OnSpotLightComponentDestroy>();

		m_CameraObserver = CreateObserver(entt::collector.update<CameraComponent>());
		m_BoundsObserver = CreateObserver(entt::collector.update<BoundsComponent>().update<TransformComponent>());
		m_PhysicsObserver = CreateObserver(entt::collector.update<TransformComponent>().where<RigidBodyComponent>());
	}

	void LevelObserverSystem::OnUnbind()
	{
		UnbindOnComponentAdded<CameraComponent>();
		UnbindOnComponentDestroy<CameraComponent>();

		UnbindOnComponentAdded<MeshRendererComponent>();
		UnbindOnComponentUpdated<MeshRendererComponent>();
		UnbindOnComponentDestroy<MeshRendererComponent>();

		UnbindOnComponentUpdated<ScriptComponent>();
		UnbindOnComponentDestroy<ScriptComponent>();

		UnbindOnComponentAdded<DirectionalLightComponent>();
		UnbindOnComponentUpdated<DirectionalLightComponent>();
		UnbindOnComponentDestroy<DirectionalLightComponent>();

		UnbindOnComponentAdded<PointLightComponent>();
		UnbindOnComponentUpdated<PointLightComponent>();
		UnbindOnComponentDestroy<PointLightComponent>();

		UnbindOnComponentAdded<SpotLightComponent>();
		UnbindOnComponentUpdated<SpotLightComponent>();
		UnbindOnComponentDestroy<SpotLightComponent>();

		m_CameraObserver->disconnect();
		m_BoundsObserver->disconnect();
		m_PhysicsObserver->disconnect();
	}

	void LevelObserverSystem::OnUpdate(Scene& scene)
	{
		m_CameraObserver->each([&scene](const entt::entity e)
		{
			const Entity entity{ e, scene.shared_from_this() };
			auto& cameraComp = entity.GetComponent<CameraComponent>();
			cameraComp.Camera.RecalculateProjection();
		});

		m_BoundsObserver->each([&scene](const auto e)
		{
			const Entity entity{ e, scene.shared_from_this() };
			entity.UpdateBounds();
		});

		m_PhysicsObserver->each([&scene](const auto e)
		{
			if (const auto* physicsScene = scene.GetPhysicsScene())
			{
				const Entity entity{ e, scene.shared_from_this() };
				if (const auto* actor = physicsScene->GetActor(entity))
				{
					// Set transform back to physics actor
					actor->SetTransform(entity.GetTransform());
				}
			}
		});
	}

	void MeshPreviewObserverSystem::OnBind()
	{
		BindOnComponentAdded<MeshDetailComponent, &OnMeshDetailComponentAdded>();
		BindOnComponentUpdated<MeshDetailComponent, &OnMeshDetailComponentFieldChanged>();
		BindOnComponentDestroy<MeshDetailComponent, &OnMeshDetailComponentDestroy>();

		BindOnComponentAdded<DirectionalLightComponent, &OnDirectionalLightComponentAdded>();
		BindOnComponentUpdated<DirectionalLightComponent, &OnDirectionalLightComponentFieldChanged>();
		BindOnComponentDestroy<DirectionalLightComponent, &OnDirectionalLightComponentDestroy>();

		m_BoundsObserver = CreateObserver(entt::collector.update<BoundsComponent>().update<TransformComponent>());
	}

	void MeshPreviewObserverSystem::OnUnbind()
	{
		UnbindOnComponentAdded<MeshDetailComponent>();
		UnbindOnComponentUpdated<MeshDetailComponent>();
		UnbindOnComponentDestroy<MeshDetailComponent>();

		UnbindOnComponentAdded<DirectionalLightComponent>();
		UnbindOnComponentUpdated<DirectionalLightComponent>();
		UnbindOnComponentDestroy<DirectionalLightComponent>();

		m_BoundsObserver->disconnect();
	}

	void MeshPreviewObserverSystem::OnUpdate(Scene& scene)
	{
		m_BoundsObserver->each([&scene](const auto e)
		{
			const Entity entity{ e, scene.shared_from_this() };
			entity.UpdateBounds();
		});
	}

		void MaterialPreviewObserverSystem::OnBind()
	{
		BindOnComponentUpdated<MaterialDetailComponent, &OnMaterialDetailComponentFieldChanged>();

		BindOnComponentAdded<MeshRendererComponent, &OnMeshRendererComponentAdded>();
		BindOnComponentUpdated<MeshRendererComponent, &OnMeshRendererComponentFieldChanged>();
		BindOnComponentDestroy<MeshRendererComponent, &OnMeshRendererComponentDestroy>();

		BindOnComponentAdded<DirectionalLightComponent, &OnDirectionalLightComponentAdded>();
		BindOnComponentUpdated<DirectionalLightComponent, &OnDirectionalLightComponentFieldChanged>();
		BindOnComponentDestroy<DirectionalLightComponent, &OnDirectionalLightComponentDestroy>();

		m_BoundsObserver = CreateObserver(entt::collector.update<BoundsComponent>().update<TransformComponent>());
	}

	void MaterialPreviewObserverSystem::OnUnbind()
	{
		UnbindOnComponentUpdated<MaterialDetailComponent>();

		UnbindOnComponentAdded<MeshRendererComponent>();
		UnbindOnComponentUpdated<MeshRendererComponent>();
		UnbindOnComponentDestroy<MeshRendererComponent>();

		UnbindOnComponentAdded<DirectionalLightComponent>();
		UnbindOnComponentUpdated<DirectionalLightComponent>();
		UnbindOnComponentDestroy<DirectionalLightComponent>();

		m_BoundsObserver->disconnect();
	}

	void MaterialPreviewObserverSystem::OnUpdate(Scene& scene)
	{
		m_BoundsObserver->each([&scene](const auto e)
		{
			const Entity entity{ e, scene.shared_from_this() };
			entity.UpdateBounds();
		});
	}

	void TexturePreviewObserverSystem::OnBind()
	{
		BindOnComponentUpdated<TextureDetailComponent, &OnTextureDetailComponentFieldChanged>();
	}

	void TexturePreviewObserverSystem::OnUnbind()
	{
		UnbindOnComponentUpdated<TextureDetailComponent>();
	}

	void ParticleUpdateSystem::OnUpdateEditor(DeltaTime dt)
	{
		auto particleView = GetScene()->GetComponentView<ParticleSystemComponent>();
		for (const auto e : particleView)
		{
			auto [particleComp] = particleView.get(e);
			if (particleComp.Instance)
			{
				particleComp.Instance->OnUpdate(dt);
			}
		}
	}

	void ParticleUpdateSystem::OnUpdateRuntime(DeltaTime dt)
	{
		OnUpdateEditor(dt);
	}

	void ParticlePreviewUpdateSystem::OnUpdateEditor(DeltaTime dt)
	{
		auto particleView = GetScene()->GetComponentView<ParticleSystemDetailComponent>();
		for (const auto e : particleView)
		{
			auto [particleComp] = particleView.get(e);
			if (particleComp.Instance)
			{
				particleComp.Instance->OnUpdate(dt);
			}
		}
	}

	void ParticlePreviewUpdateSystem::OnUpdateRuntime(DeltaTime dt)
	{
		OnUpdateEditor(dt);
	}

	void ScriptSystem::OnUpdateRuntime(DeltaTime dt)
	{
		if (GetWorld()->IsSimulation()) return;

		auto scriptView = GetScene()->GetComponentView<ScriptComponent>();
		for (const auto e : scriptView)
		{
			const Entity entity{ e, GetScene() };
			ScriptEngine::OnUpdateEntity(entity, dt);
		}
	}

	void ScriptSystem::OnPlayStart()
	{
		ScriptEngine::SetSceneContext(GetScene());
		auto scriptView = GetScene()->GetComponentView<ScriptComponent>();
		for (const auto e : scriptView)
		{
			const Entity entity{ e, GetScene() };
			ScriptEngine::InstantiateEntityClass(entity);
			ScriptEngine::OnCreateEntity(entity);
		}
	}

	void ScriptSystem::OnPlayStop()
	{
		ScriptEngine::SetSceneContext(GetScene());
	}

	void PhysicsSystem::OnUpdateRuntime(DeltaTime dt)
	{
		GetScene()->GetPhysicsScene()->Simulate(dt);
	}

	void PhysicsSystem::OnPlayStart()
	{
		const auto scene = GetScene();
		auto* physicsScene = scene->CreatePhysicsScene();
		auto rigidBodyView = scene->GetComponentView<RigidBodyComponent>();
		for (const auto e : rigidBodyView)
		{
			const Entity entity{ e, scene };
			physicsScene->CreateActor(entity);
		}

#ifdef ZE_DEBUG
		const auto& settings = PhysicsEngine::GetSettings();
		if (settings.bDebugOnPlay)
		{
			if (settings.DebugType == PhysXDebugType::LiveDebug)
			{
				PhysXDebugger::StartDebugging();
			}
			else
			{
				// TODO: PhysX debugger output path
				PhysXDebugger::StartDebugging(AssetRegistry::GetProjectDirectory());
			}
		}
#endif
	}

	void PhysicsSystem::OnPlayStop()
	{
#ifdef ZE_DEBUG
		if (PhysicsEngine::GetSettings().bDebugOnPlay)
		{
			PhysXDebugger::StopDebugging();
		}
		GetScene()->DestroyPhysicsScene();
#endif
	}

	void PhysicsSystem::OnSimulationStart()
	{
		OnPlayStart();
	}

	void PhysicsSystem::OnSimulationStop()
	{
		OnPlayStop();
	}

	void PhysicsSystem2D::OnUpdateRuntime(DeltaTime dt)
	{
		const I32 velocityIterations = 6;
		const I32 positionIterations = 2;
		m_PhysicsWorld->Step(dt, velocityIterations, positionIterations);

		auto rb2dGroup = GetScene()->GetComponentGroup<RigidBody2DComponent>(IncludeComponents<TransformComponent>);
		for (const auto e : rb2dGroup)
		{
			auto [rb2dComp, transformComp] = rb2dGroup.get(e);
			// Retrieve transform from Box2D
			const b2Body* body = static_cast<b2Body*>(rb2dComp.RuntimeBody);
			const auto& position = body->GetPosition();
			transformComp.Translation.x = position.x;
			transformComp.Translation.y = position.y;
			transformComp.Rotation.z = glm::degrees(body->GetAngle());
		}
	}

	static b2BodyType RigidBody2DTypeToBox2DBody(RigidBody2DComponent::BodyType bodyType)
	{
		switch (bodyType)
		{
			case RigidBody2DComponent::BodyType::Static:	return b2_staticBody;
			case RigidBody2DComponent::BodyType::Dynamic:	return b2_dynamicBody;
			case RigidBody2DComponent::BodyType::Kinematic:	return b2_kinematicBody;
		}

		ZE_CORE_ASSERT(false, "Unknown body type!");
		return b2_staticBody;
	}

	void PhysicsSystem2D::OnPlayStart()
	{
		const b2Vec2 gravity = { 0.0f, -9.8f };
		m_PhysicsWorld = new b2World(gravity);
		auto rb2dGroup = GetScene()->GetComponentGroup<RigidBody2DComponent>(IncludeComponents<TransformComponent>);
		for (const auto e : rb2dGroup)
		{
			auto [rb2dComp, transformComp] = rb2dGroup.get(e);
			b2BodyDef bodyDef;
			bodyDef.type = RigidBody2DTypeToBox2DBody(rb2dComp.Type);
			bodyDef.position.Set(transformComp.Translation.x, transformComp.Translation.y);
			bodyDef.angle = glm::radians(transformComp.Rotation.z);

			b2Body* body = m_PhysicsWorld->CreateBody(&bodyDef);
			body->SetFixedRotation(rb2dComp.bFixedRotation);
			rb2dComp.RuntimeBody = body;

			const Entity entity{ e, GetScene() };
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
		}
	}

	void PhysicsSystem2D::OnPlayStop()
	{
		delete m_PhysicsWorld;
		m_PhysicsWorld = nullptr;
	}

	void PhysicsSystem2D::OnSimulationStart()
	{
		OnPlayStart();
	}

	void PhysicsSystem2D::OnSimulationStop()
	{
		OnPlayStop();
	}

	void CommandSystem::OnPlayStart()
	{
		Console::Get().ResetAllVariableValues(CommandType::EditOnly);
	}

	void CommandSystem::OnPlayStop()
	{
		Console::Get().ResetAllVariableValues(CommandType::RuntimeOnly);
	}

	void CommandSystem::OnSimulationStart()
	{
		OnPlayStart();
	}

	void CommandSystem::OnSimulationStop()
	{
		OnPlayStop();
	}

}

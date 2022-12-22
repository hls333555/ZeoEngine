#include "ZEpch.h"
#include "Engine/GameFramework/Systems.h"

#include <box2d/b2_world.h>
#include <box2d/b2_body.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_circle_shape.h>

#include "Engine/GameFramework/Tags.h"
#include "Engine/Core/Console.h"
#include "Engine/GameFramework/Components.h"
#include "Engine/Renderer/SceneRenderer.h"
#include "Engine/Scripting/ScriptEngine.h"
#include "Engine/GameFramework/World.h"
#include "Engine/Physics/PhysicsEngine.h"
#include "Engine/Physics/PhysXCharacterController.h"
#include "Engine/Physics/PhysXScene.h"
#include "Engine/Physics/PhysXShapes.h"
#include "Engine/Renderer/RenderPass.h"

namespace ZeoEngine {

	ISystem::ISystem(WorldBase* world)
		: m_World(world)
	{
	}

	void SystemBase::OnCreate()
	{
		GetWorld()->m_OnActiveSceneChanged.connect<&SystemBase::OnActiveSceneChanged>(this);
	}

	void SystemBase::OnUpdate(DeltaTime dt)
	{
		OnUpdateComponentObservers();
	}

	void SystemBase::OnDestroy()
	{
		GetWorld()->m_OnActiveSceneChanged.disconnect(this);

		for (const auto& binder : m_ComponentEventBinders)
		{
			binder->OnUnbind(GetScene().get());
		}

		for (const auto& observer : m_ComponentObservers)
		{
			observer->OnUnbind();
		}
	}

	void SystemBase::OnActiveSceneChanged(Scene* scene, Scene* lastScene) const
	{
		for (const auto& binder : m_ComponentEventBinders)
		{
			binder->OnUnbind(lastScene);
			binder->OnBind(scene);
		}

		for (const auto& observer : m_ComponentObservers)
		{
			observer->OnUnbind();
			observer->OnBind(*scene);
		}
	}

	void SystemBase::OnUpdateComponentObservers() const
	{
		ZE_PROFILE_FUNC();

		for (const auto& observer : m_ComponentObservers)
		{
			observer->OnUpdate(*GetScene());
		}
	}

	void TransformSystem::OnCreate()
	{
		SystemBase::OnCreate();

		RegisterComponentEventBinder<ComponentUpdatedBinder<TransformComponent, &TransformSystem::OnTransformComponentUpdated, TransformSystem>>(this);
	}

	// Sync transforms between world and local for all child entities
	// If world transform is changed, LocalTransformDirty tag is added so that local transform will be calculated based on the world transform
	// If local transform is changed, LocalTransformDirty tag is removed so that world transform will be calculated based on the local transform
	// In this case, only the most recent transform change will be taken into account which means if we change local transform first and then change world transform, the previous change will be ignored
	// Changes to any transform will add the AnyTransformDirty tag which will be removed in physics update
	// We do not update root parent entities here by referencing WorldTransformComponent which is only added to child entities, because their local transforms are also the world ones
	void TransformSystem::OnUpdate(DeltaTime dt)
	{
		ZE_PROFILE_FUNC();

		SystemBase::OnUpdate(dt);

		const auto scene = GetScene();
		auto transformGroup = scene->GetComponentGroup<TransformComponent, WorldTransformComponent, entt::tag<Tag::AnyTransformDirty>>();
		// Sort entities to ensure parent ones are updated before their children as child transforms rely on its parent's
		{
			ZE_PROFILE_FUNC("TansformSystem::SortTransformDirtyEntities")
			transformGroup.sort([&scene](const auto lhs, const auto rhs)
		   {
			   const Entity lEntity{ lhs, scene };
			   const Entity rEntity{ rhs, scene };
			   return lEntity.IsAncestorOf(rEntity);
		   });
		}
		for (const auto e : transformGroup)
		{
			Entity entity{ e, scene };
			auto [transformComp, worldTransformComp] = transformGroup.get<TransformComponent, WorldTransformComponent>(e);
			if (entity.HasTag<Tag::LocalTransformDirty>())
			{
				const Entity parent = entity.GetParentEntity();
				const Mat4 parentTransform = parent.GetWorldTransform();
				const Mat4 localTransform = glm::inverse(parentTransform) * worldTransformComp.GetTransform();
				transformComp.SetTransform(localTransform);

				entity.RemoveTag<Tag::LocalTransformDirty>();
			}
			else
			{
				const Entity parent = entity.GetParentEntity();
				const Mat4 parentTransform = parent.GetWorldTransform();
				const Mat4 worldTransform = parentTransform * transformComp.GetTransform();
				worldTransformComp.SetTransform(worldTransform);
			}
		}
	}

	void TransformSystem::OnTransformComponentUpdated(Scene& scene, entt::entity e) const
	{
		const Entity entity{ e, scene.shared_from_this() };
		// WorldTransformComponent is restored and all transforms are properly deserialized so that we do not need to queue a pending update on scene deserialization
		if (entity.HasTag<Tag::IsDeserializing>()) return;

		// We need mark dirty here as local transform can be edited in inspector which will directly modify TRS values instead of calling our setters
		entity.MarkWorldTransformDirty();
	}

	// Sync changed transforms back to child entities after physics simulation
	void PostPhysicsTransformSystem::OnUpdate(DeltaTime dt)
	{
		ZE_PROFILE_FUNC();

		SystemBase::OnUpdate(dt);

		const auto scene = GetScene();
		auto transformGroup = scene->GetComponentGroup<TransformComponent, WorldTransformComponent, entt::tag<Tag::AnyTransformDirty>>();
		// We do not sort here based on an assumption that no entities will be added or removed during physics update
		for (const auto e : transformGroup)
		{
			Entity entity{ e, scene };
			auto [transformComp, worldTransformComp] = transformGroup.get<TransformComponent, WorldTransformComponent>(e);
			if (entity.HasTag<Tag::LocalTransformDirty>())
			{
				const Entity parent = entity.GetParentEntity();
				const Mat4 parentTransform = parent.GetWorldTransform();
				const Mat4 localTransform = glm::inverse(parentTransform) * worldTransformComp.GetTransform();
				transformComp.SetTransform(localTransform);

				entity.RemoveTag<Tag::LocalTransformDirty>();
			}
			else // Certain child entities attached to physics entities need update their world transforms accordingly
			{
				const Entity parent = entity.GetParentEntity();
				const Mat4 parentTransform = parent.GetWorldTransform();
				const Mat4 worldTransform = parentTransform * transformComp.GetTransform();
				worldTransformComp.SetTransform(worldTransform);
			}
		}

		GetScene()->ClearTags<Tag::AnyTransformDirty>();
	}

	static void OnCameraObserverUpdate(Scene& scene, Entity entity)
	{
		auto& cameraComp = entity.GetComponent<CameraComponent>();
		cameraComp.Camera.RecalculateProjection();
	}

	void CameraSystem::OnCreate()
	{
		SystemBase::OnCreate();

		RegisterComponentEventBinder<ComponentAddedBinder<CameraComponent, &CameraSystem::OnCameraComponentAdded, CameraSystem>>(this);
		RegisterComponentEventBinder<ComponentDestroyBinder<CameraComponent, &CameraSystem::OnCameraComponentDestroy, CameraSystem>>(this);
		RegisterComponentObserver(&OnCameraObserverUpdate, entt::collector.update<CameraComponent>());
	}

	void CameraSystem::OnCameraComponentAdded(Scene& scene, entt::entity e) const
	{
		Entity entity{ e, scene.shared_from_this() };
		auto& billboardComp = entity.AddComponent<BillboardComponent>();
		billboardComp.TextureAsset = AssetLibrary::LoadAsset<Texture2D>("assets/textures/icons/Camera.png.zasset");
	}

	void CameraSystem::OnCameraComponentDestroy(Scene& scene, entt::entity e) const
	{
		Entity entity{ e, scene.shared_from_this() };
		entity.RemoveComponentIfExist<BillboardComponent>();
	}

	void MeshSystem::OnCreate()
	{
		SystemBase::OnCreate();

		RegisterComponentEventBinder<ComponentAddedBinder<MeshRendererComponent, &MeshSystem::OnMeshRendererComponentAdded, MeshSystem>>(this);
		RegisterComponentEventBinder<ComponentUpdatedBinder<MeshRendererComponent, &MeshSystem::OnMeshRendererComponentUpdated, MeshSystem>>(this);
		RegisterComponentEventBinder<ComponentDestroyBinder<MeshRendererComponent, &MeshSystem::OnMeshRendererComponentDestroy, MeshSystem>>(this);
	}

	void MeshSystem::OnMeshRendererComponentAdded(Scene& scene, entt::entity e) const
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
			const auto& meshComp = entity.GetComponent<MeshRendererComponent>();
			const auto mesh = AssetLibrary::LoadAsset<Mesh>(meshComp.MeshAsset);
			return mesh ? mesh->GetBounds().TransformBy(entity.GetWorldTransform()) : BoxSphereBounds{};
		};
	}

	void MeshSystem::OnMeshRendererComponentUpdated(Scene& scene, entt::entity e) const
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

	void MeshSystem::OnMeshRendererComponentDestroy(Scene& scene, entt::entity e) const
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

	void MeshPreviewSystem::OnCreate()
	{
		SystemBase::OnCreate();

		RegisterComponentEventBinder<ComponentAddedBinder<MeshDetailComponent, &MeshPreviewSystem::OnMeshDetailComponentAdded, MeshPreviewSystem>>(this);
		RegisterComponentEventBinder<ComponentUpdatedBinder<MeshDetailComponent, &MeshPreviewSystem::OnMeshDetailComponentUpdated, MeshPreviewSystem>>(this);
		RegisterComponentEventBinder<ComponentDestroyBinder<MeshDetailComponent, &MeshPreviewSystem::OnMeshDetailComponentDestroy, MeshPreviewSystem>>(this);
	}

	void MeshPreviewSystem::OnMeshDetailComponentAdded(Scene& scene, entt::entity e) const
	{
		const Entity entity{ e, scene.shared_from_this() };
		auto& boundsComp = entity.GetComponent<BoundsComponent>();
		boundsComp.BoundsCalculationFuncs[entt::type_hash<MeshDetailComponent>::value()] = [](Entity entity)
		{
			const auto& meshComp = entity.GetComponent<MeshDetailComponent>();
			const auto& mesh = meshComp.LoadedMesh;
			return mesh ? mesh->GetBounds().TransformBy(entity.GetWorldTransform()) : BoxSphereBounds{};
		};
	}

	void MeshPreviewSystem::OnMeshDetailComponentUpdated(Scene& scene, entt::entity e) const
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

	void MeshPreviewSystem::OnMeshDetailComponentDestroy(Scene& scene, entt::entity e) const
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

	void MaterialPreviewSystem::OnCreate()
	{
		SystemBase::OnCreate();

		RegisterComponentEventBinder<ComponentUpdatedBinder<MaterialDetailComponent, &MaterialPreviewSystem::OnMaterialDetailComponentUpdated, MaterialPreviewSystem>>(this);
	}

	void MaterialPreviewSystem::OnMaterialDetailComponentUpdated(Scene& scene, entt::entity e) const
	{
		const Entity entity{ e, scene.shared_from_this() };
		const auto& materialComp = entity.GetComponent<MaterialDetailComponent>();
		const auto& changeComp = entity.GetComponent<FieldChangeComponent>();
		if (changeComp.FieldID == "ShaderAsset"_hs)
		{
			materialComp.LoadedMaterial->NotifyShaderAssetChange();
		}
	}

	void TexturePreviewSystem::OnCreate()
	{
		SystemBase::OnCreate();

		RegisterComponentEventBinder<ComponentUpdatedBinder<TextureDetailComponent, &TexturePreviewSystem::OnTextureDetailComponentUpdated, TexturePreviewSystem>>(this);
	}

	void TexturePreviewSystem::OnTextureDetailComponentUpdated(Scene& scene, entt::entity e) const
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

	void DirectionalLightSystem::OnCreate()
	{
		SystemBase::OnCreate();

		RegisterComponentEventBinder<ComponentAddedBinder<DirectionalLightComponent, &DirectionalLightSystem::OnDirectionalLightComponentAdded, DirectionalLightSystem>>(this);
		RegisterComponentEventBinder<ComponentUpdatedBinder<DirectionalLightComponent, &DirectionalLightSystem::OnDirectionalLightComponentUpdated, DirectionalLightSystem>>(this);
		RegisterComponentEventBinder<ComponentDestroyBinder<DirectionalLightComponent, &DirectionalLightSystem::OnDirectionalLightComponentDestroy, DirectionalLightSystem>>(this);
	}

	void DirectionalLightSystem::OnDirectionalLightComponentAdded(Scene& scene, entt::entity e) const
	{
		Entity entity{ e, scene.shared_from_this() };
		auto& billboardComp = entity.AddComponent<BillboardComponent>();
		billboardComp.TextureAsset = AssetLibrary::LoadAsset<Texture2D>("assets/textures/icons/DirectionalLight.png.zasset");
		entity.PatchComponent<BoundsComponent>([](BoundsComponent& boundsComp)
		{
			boundsComp.BoundsCalculationFuncs[entt::type_hash<DirectionalLightComponent>::value()] = [](Entity entity)
			{
				const Sphere sphere{ entity.GetWorldTranslation(), 0.0f };
				return sphere;
			};
		});
	}

	void DirectionalLightSystem::OnDirectionalLightComponentUpdated(Scene& scene, entt::entity e) const
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

	void DirectionalLightSystem::OnDirectionalLightComponentDestroy(Scene& scene, entt::entity e) const
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

	void PointLightSystem::OnCreate()
	{
		SystemBase::OnCreate();

		RegisterComponentEventBinder<ComponentAddedBinder<PointLightComponent, &PointLightSystem::OnPointLightComponentAdded, PointLightSystem>>(this);
		RegisterComponentEventBinder<ComponentUpdatedBinder<PointLightComponent, &PointLightSystem::OnPointLightComponentUpdated, PointLightSystem>>(this);
		RegisterComponentEventBinder<ComponentDestroyBinder<PointLightComponent, &PointLightSystem::OnPointLightComponentDestroy, PointLightSystem>>(this);
	}

	void PointLightSystem::OnPointLightComponentAdded(Scene& scene, entt::entity e) const
	{
		Entity entity{ e, scene.shared_from_this() };
		auto& billboardComp = entity.AddComponent<BillboardComponent>();
		billboardComp.TextureAsset = AssetLibrary::LoadAsset<Texture2D>("assets/textures/icons/PointLight.png.zasset");
		entity.PatchComponent<BoundsComponent>([](BoundsComponent& boundsComp)
		{
			boundsComp.BoundsCalculationFuncs[entt::type_hash<PointLightComponent>::value()] = [](Entity entity)
			{
				const auto& lightComp = entity.GetComponent<PointLightComponent>();
				const Sphere sphere{ entity.GetWorldTranslation(), lightComp.Range };
				return sphere;
			};
		});
	}

	void PointLightSystem::OnPointLightComponentUpdated(Scene& scene, entt::entity e) const
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

	void PointLightSystem::OnPointLightComponentDestroy(Scene& scene, entt::entity e) const
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

	void SpotLightSystem::OnCreate()
	{
		SystemBase::OnCreate();

		RegisterComponentEventBinder<ComponentAddedBinder<SpotLightComponent, &SpotLightSystem::OnSpotLightComponentAdded, SpotLightSystem>>(this);
		RegisterComponentEventBinder<ComponentUpdatedBinder<SpotLightComponent, &SpotLightSystem::OnSpotLightComponentUpdated, SpotLightSystem>>(this);
		RegisterComponentEventBinder<ComponentDestroyBinder<SpotLightComponent, &SpotLightSystem::OnSpotLightComponentDestroy, SpotLightSystem>>(this);
	}

	void SpotLightSystem::OnSpotLightComponentAdded(Scene& scene, entt::entity e) const
	{
		Entity entity{ e, scene.shared_from_this() };
		auto& billboardComp = entity.AddComponent<BillboardComponent>();
		billboardComp.TextureAsset = AssetLibrary::LoadAsset<Texture2D>("assets/textures/icons/SpotLight.png.zasset");
		entity.PatchComponent<BoundsComponent>([](BoundsComponent& boundsComp)
		{
			boundsComp.BoundsCalculationFuncs[entt::type_hash<SpotLightComponent>::value()] = [](Entity entity)
			{
			   const auto& lightComp = entity.GetComponent<SpotLightComponent>();
			   const Sphere sphere{ entity.GetWorldTranslation(), lightComp.Range * 0.5f };
			   return sphere;
			};
		});
	}

	void SpotLightSystem::OnSpotLightComponentUpdated(Scene& scene, entt::entity e) const
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

	void SpotLightSystem::OnSpotLightComponentDestroy(Scene& scene, entt::entity e) const
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

	static void OnBoundsObserverUpdate(Scene& scene, Entity entity)
	{
		entity.UpdateBounds();
	}

	void BoundsSystem::OnCreate()
	{
		SystemBase::OnCreate();

		RegisterComponentObserver(&OnBoundsObserverUpdate, entt::collector.update<BoundsComponent>().update<TransformComponent>().update<WorldTransformComponent>());
	}

	void ParticleUpdateSystem::OnUpdate(DeltaTime dt)
	{
		SystemBase::OnUpdate(dt);

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

	void ParticlePreviewUpdateSystem::OnUpdate(DeltaTime dt)
	{
		ParticleUpdateSystem::OnUpdate(dt);

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

	void ScriptSystem::OnCreate()
	{
		SystemBase::OnCreate();

		RegisterComponentEventBinder<ComponentUpdatedBinder<ScriptComponent, &ScriptSystem::OnScriptComponentUpdated, ScriptSystem>>(this);
		RegisterComponentEventBinder<ComponentDestroyBinder<ScriptComponent, &ScriptSystem::OnScriptComponentDestroy, ScriptSystem>>(this);
	}

	void ScriptSystem::OnUpdate(DeltaTime dt)
	{
		ZE_PROFILE_FUNC();

		SystemBase::OnUpdate(dt);

		if (!GetWorld()->IsRunning()) return;
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
		ZE_PROFILE_FUNC();

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
		ZE_PROFILE_FUNC();

		ScriptEngine::SetSceneContext(GetScene());
	}

	void ScriptSystem::OnScriptComponentUpdated(Scene& scene, entt::entity e) const
	{
		const Entity entity{ e, scene.shared_from_this() };
		const auto& changeComp = entity.GetComponent<FieldChangeComponent>();
		if (changeComp.FieldID == "ClassName"_hs)
		{
			ScriptEngine::OnScriptClassChanged(entity);
		}
	}

	void ScriptSystem::OnScriptComponentDestroy(Scene& scene, entt::entity e) const
	{
		const Entity entity{ e, scene.shared_from_this() };
		ScriptEngine::OnDestroyEntity(entity);
	}

	void PhysicsSystem::OnUpdate(DeltaTime dt)
	{
		ZE_PROFILE_FUNC();

		SystemBase::OnUpdate(dt);

		const auto scene = GetScene();
		if (!GetWorld()->IsRunning())
		{
			// Physics does not exist yet, remove the tag and return
			scene->ClearTags<Tag::AnyTransformDirty>();
			return;
		}

		auto* physicsScene = scene->GetPhysicsScene();
		if (!physicsScene) return;

		// Sync changed transforms to physics
		auto transformGroup = scene->GetComponentGroup<TransformComponent, entt::tag<Tag::AnyTransformDirty>>();
		for (const auto e : transformGroup)
		{
			Entity entity{ e, GetScene() };
			const Vec3& translation = entity.GetWorldTranslation();
			const Vec3 rotation = entity.GetWorldRotation();

			// Physics actor
			if (entity.HasComponent<RigidBodyComponent>())
			{
				const auto* actor = physicsScene->GetActor(entity);
				actor->SetTransform(translation, rotation);
			}
			// Physics collider
			else if (entity.HasTag<Tag::ChildCollider>())
			{
				Entity current = entity;
				Entity parent;
				Mat4 transform = entity.GetTransform();
				while ((parent = current.GetParentEntity()))
				{
					if (parent.HasComponent<RigidBodyComponent>())
					{
						break;
					}

					current = parent;
					transform = parent.GetTransform() * transform;
				}

				if (const auto* actor = physicsScene->GetActor(parent))
				{
					const auto* colliders = actor->GetCollidersByEntity(entity);
					if (colliders)
					{
						for (const auto& collider : *colliders)
						{
							collider->SetTransform(transform);
						}
					}
				}
			}
			// Physics character controller
			if (entity.HasComponent<CharacterControllerComponent>())
			{
				const auto& controllerComp = entity.GetComponent<CharacterControllerComponent>();
				const auto* controller = physicsScene->GetCharacterController(entity);
				controller->SetTranslation(translation + controllerComp.Offset);
			}

			entity.RemoveTag<Tag::AnyTransformDirty>();
		}

		// Physics simulation
		physicsScene->Simulate(dt);
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

		ValidateColliders();

		auto characterControllerView = scene->GetComponentView<CharacterControllerComponent>();
		for (const auto e : characterControllerView)
		{
			const Entity entity{ e, scene };
			physicsScene->CreateCharacterController(entity);
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

	void PhysicsSystem::ValidateColliders()
	{
		const auto scene = GetScene();
		auto boxView = scene->GetComponentView<BoxColliderComponent>(ExcludeComponents<RigidBodyComponent>);
		for (const auto e : boxView)
		{
			const Entity entity{ e, scene };
			if (!entity.HasTag<Tag::ChildCollider>())
			{
				ZE_CORE_WARN("Entity '{0}' has BoxColliderComponent but no RigidBodyComponent, so it will not participate in any physics interaction", entity.GetName());
			}
		}

		auto sphereView = scene->GetComponentView<SphereColliderComponent>(ExcludeComponents<RigidBodyComponent>);
		for (const auto e : sphereView)
		{
			const Entity entity{ e, scene };
			if (!entity.HasTag<Tag::ChildCollider>())
			{
				ZE_CORE_WARN("Entity '{0}' has SphereColliderComponent but no RigidBodyComponent, so it will not participate in any physics interaction", entity.GetName());
			}
		}

		auto capsuleView = scene->GetComponentView<CapsuleColliderComponent>(ExcludeComponents<RigidBodyComponent>);
		for (const auto e : capsuleView)
		{
			const Entity entity{ e, scene };
			if (!entity.HasTag<Tag::ChildCollider>())
			{
				ZE_CORE_WARN("Entity '{0}' has CapsuleColliderComponent but no RigidBodyComponent, so it will not participate in any physics interaction", entity.GetName());
			}
		}
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

	void PhysicsSystem2D::OnUpdate(DeltaTime dt)
	{
		PhysicsSystem::OnUpdate(dt);

		if (!GetWorld()->IsRunning()) return;

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
		Console::Get().ResetAllVariableValues();
	}

	void CommandSystem::OnPlayStop()
	{
		Console::Get().ResetAllVariableValues();
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

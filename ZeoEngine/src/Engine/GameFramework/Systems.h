#pragma once

#include "Engine/GameFramework/World.h"
#include "Engine/Core/DeltaTime.h"

class b2World;

namespace ZeoEngine {

	class SceneCamera;

	class ISystem
	{
	public:
		explicit ISystem(WorldBase* world);
		virtual ~ISystem() = default;

	protected:
		WorldBase* GetWorld() const { return m_World; }
		Ref<Scene> GetScene() const { return GetWorld()->GetActiveScene(); }

	private:
		WorldBase* m_World = nullptr;
	};

	class IComponentEventBinder
	{
	public:
		virtual ~IComponentEventBinder() = default;

		virtual void OnBind(Scene* scene) = 0;
		virtual void OnUnbind(Scene* scene) = 0;
	};

	template<typename Component, auto Func, typename InstanceType>
	class ComponentAddedBinder : public IComponentEventBinder
	{
	public:
		explicit ComponentAddedBinder(InstanceType* instance)
			: m_Instance(instance)
		{
		}

		virtual void OnBind(Scene* scene) override
		{
			scene->on_construct<Component>().template connect<Func>(m_Instance);
			
		}
		virtual void OnUnbind(Scene* scene) override
		{
			scene->on_construct<Component>().disconnect();
		}

	private:
		InstanceType* m_Instance = nullptr;
	};

	template<typename Component, auto Func, typename InstanceType>
	class ComponentUpdatedBinder : public IComponentEventBinder
	{
	public:
		explicit ComponentUpdatedBinder(InstanceType* instance)
			: m_Instance(instance)
		{
		}

		virtual void OnBind(Scene* scene) override
		{
			scene->on_update<Component>().template connect<Func>(m_Instance);
		}
		virtual void OnUnbind(Scene* scene) override
		{
			scene->on_update<Component>().disconnect();
		}

	private:
		InstanceType* m_Instance = nullptr;
	};

	template<typename Component, auto Func, typename InstanceType>
	class ComponentDestroyBinder : public IComponentEventBinder
	{
	public:
		explicit ComponentDestroyBinder(InstanceType* instance)
			: m_Instance(instance)
		{
		}

		virtual void OnBind(Scene* scene) override
		{
			scene->on_destroy<Component>().template connect<Func>(m_Instance);
		}
		virtual void OnUnbind(Scene* scene) override
		{
			scene->on_destroy<Component>().disconnect();
		}

	private:
		InstanceType* m_Instance = nullptr;
	};

	class IComponentObserver
	{
	public:
		virtual ~IComponentObserver() = default;

		virtual void OnBind(Scene& scene) = 0;
		virtual void OnUpdate(Scene& scene) = 0;
		virtual void OnUnbind() = 0;
	};

	using ObserverUpdateFunc = void(*)(Scene& scene, Entity entity);

	template<typename... Matcher>
	class ComponentObserver : public entt::observer, public IComponentObserver
	{
	public:
		ComponentObserver(ObserverUpdateFunc updateFunc, entt::basic_collector<Matcher...>)
			: m_UpdateFunc(updateFunc)
		{
		}

		virtual void OnBind(Scene& scene) override
		{
			connect(scene, entt::basic_collector<Matcher...>{});
		}
		virtual void OnUpdate(Scene& scene) override
		{
			for (const auto e : *this)
			{
				const Entity entity{ e, scene.shared_from_this() };
				m_UpdateFunc(scene, entity);
			}
			clear();
		}
		virtual void OnUnbind() override
		{
			disconnect();
		}

	private:
		ObserverUpdateFunc m_UpdateFunc;
	};

	class SystemBase : public ISystem
	{
	public:
		using ISystem::ISystem;

		virtual void OnCreate();
		virtual void OnUpdate(DeltaTime dt);
		virtual void OnDestroy();

		virtual void OnPlayStart() {}
		virtual void OnPlayStop() {}
		virtual void OnSimulationStart() {}
		virtual void OnSimulationStop() {}

	protected:
		template<typename Binder, typename Type>
		void RegisterComponentEventBinder(Type* instance)
		{
			static_assert(std::is_base_of_v<IComponentEventBinder, Binder>, "Binder class must be ComponentAddedBinder, ComponentUpdatedBinder or ComponentDestroyBinder!");

			Scope<IComponentEventBinder> binder = CreateScope<Binder>(instance);
			binder->OnBind(GetScene().get());
			m_ComponentEventBinders.emplace_back(std::move(binder));
		}

		template<typename... Matcher>
		void RegisterComponentObserver(ObserverUpdateFunc updateFunc, entt::basic_collector<Matcher...>)
		{
			Scope<IComponentObserver> observer = CreateScope<ComponentObserver<Matcher...>>(updateFunc, entt::basic_collector<Matcher...>{});
			observer->OnBind(*GetScene());
			m_ComponentObservers.emplace_back(std::move(observer));
		}

	private:
		void OnActiveSceneChanged(Scene* scene, Scene* lastScene) const;
		void OnUpdateComponentObservers() const;

	private:
		std::vector<Scope<IComponentEventBinder>> m_ComponentEventBinders;
		std::vector<Scope<IComponentObserver>> m_ComponentObservers;
	};

	class TransformSystem : public SystemBase
	{
	public:
		using SystemBase::SystemBase;

		virtual void OnCreate() override;
		virtual void OnUpdate(DeltaTime dt) override;

	private:
		void OnTransformComponentUpdated(Scene& scene, entt::entity e) const;
	};

	class PostPhysicsTransformSystem : public SystemBase
	{
	public:
		using SystemBase::SystemBase;

		virtual void OnUpdate(DeltaTime dt) override;
	};

	class CameraSystem : public SystemBase
	{
	public:
		using SystemBase::SystemBase;

		virtual void OnCreate() override;

	protected:
		virtual void OnCameraComponentAdded(Scene& scene, entt::entity e) const;
	private:
		void OnCameraComponentDestroy(Scene& scene, entt::entity e) const;
	};

	class MeshSystem : public SystemBase
	{
	public:
		using SystemBase::SystemBase;

		virtual void OnCreate() override;

	private:
		void OnMeshRendererComponentAdded(Scene& scene, entt::entity e) const;
		void OnMeshRendererComponentUpdated(Scene& scene, entt::entity e) const;
		void OnMeshRendererComponentDestroy(Scene& scene, entt::entity e) const;
	};

	class MeshPreviewSystem : public SystemBase
	{
	public:
		using SystemBase::SystemBase;

		virtual void OnCreate() override;

	private:
		void OnMeshDetailComponentAdded(Scene& scene, entt::entity e) const;
		void OnMeshDetailComponentUpdated(Scene& scene, entt::entity e) const;
		void OnMeshDetailComponentDestroy(Scene& scene, entt::entity e) const;
	};

	class MaterialPreviewSystem : public SystemBase
	{
	public:
		using SystemBase::SystemBase;

		virtual void OnCreate() override;

	private:
		void OnMaterialDetailComponentUpdated(Scene& scene, entt::entity e) const;
	};

	class TexturePreviewSystem : public SystemBase
	{
	public:
		using SystemBase::SystemBase;

		virtual void OnCreate() override;

	private:
		void OnTextureDetailComponentUpdated(Scene& scene, entt::entity e) const;
	};

	class DirectionalLightSystem : public SystemBase
	{
	public:
		using SystemBase::SystemBase;

		virtual void OnCreate() override;

	private:
		void OnDirectionalLightComponentAdded(Scene& scene, entt::entity e) const;
		void OnDirectionalLightComponentUpdated(Scene& scene, entt::entity e) const;
		void OnDirectionalLightComponentDestroy(Scene& scene, entt::entity e) const;
	};

	class PointLightSystem : public SystemBase
	{
	public:
		using SystemBase::SystemBase;

		virtual void OnCreate() override;

	private:
		void OnPointLightComponentAdded(Scene& scene, entt::entity e) const;
		void OnPointLightComponentUpdated(Scene& scene, entt::entity e) const;
		void OnPointLightComponentDestroy(Scene& scene, entt::entity e) const;
	};

	class SpotLightSystem : public SystemBase
	{
	public:
		using SystemBase::SystemBase;

		virtual void OnCreate() override;

	private:
		void OnSpotLightComponentAdded(Scene& scene, entt::entity e) const;
		void OnSpotLightComponentUpdated(Scene& scene, entt::entity e) const;
		void OnSpotLightComponentDestroy(Scene& scene, entt::entity e) const;
	};

	class BoundsSystem : public SystemBase
	{
	public:
		using SystemBase::SystemBase;

		virtual void OnCreate() override;
	};

	class ParticleUpdateSystem : public SystemBase
	{
	public:
		using SystemBase::SystemBase;

		virtual void OnUpdate(DeltaTime dt) override;
	};

	class ParticlePreviewUpdateSystem : public ParticleUpdateSystem
	{
	public:
		using ParticleUpdateSystem::ParticleUpdateSystem;

		virtual void OnUpdate(DeltaTime dt) override;
	};

	class ScriptSystem : public SystemBase
	{
	public:
		using SystemBase::SystemBase;

		virtual void OnCreate() override;
		virtual void OnUpdate(DeltaTime dt) override;

		virtual void OnPlayStart() override;
		virtual void OnPlayStop() override;

	private:
		void OnScriptComponentUpdated(Scene& scene, entt::entity e) const;
		void OnScriptComponentDestroy(Scene& scene, entt::entity e) const;
	};

	class PrePhysicsScriptSystem : public SystemBase
	{
	public:
		using SystemBase::SystemBase;

		virtual void OnUpdate(DeltaTime dt) override;
	};

	class PostPhysicsScriptSystem : public SystemBase
	{
	public:
		using SystemBase::SystemBase;

		virtual void OnUpdate(DeltaTime dt) override;
	};

	class PhysicsSystem : public SystemBase
	{
	public:
		using SystemBase::SystemBase;

		virtual void OnUpdate(DeltaTime dt) override;

		virtual void OnPlayStart() override;
		virtual void OnPlayStop() override;
		virtual void OnSimulationStart() override;
		virtual void OnSimulationStop() override;

	private:
		void ValidateColliders();
	};

	class PhysicsSystem2D : public PhysicsSystem
	{
	public:
		using PhysicsSystem::PhysicsSystem;

		virtual void OnUpdate(DeltaTime dt) override;

		virtual void OnPlayStart() override;
		virtual void OnPlayStop() override;
		virtual void OnSimulationStart() override;
		virtual void OnSimulationStop() override;

	private:
		b2World* m_PhysicsWorld = nullptr; // TODO: Move to scene
	};

	class CommandSystem : public SystemBase
	{
	public:
		using SystemBase::SystemBase;

		virtual void OnPlayStart() override;
		virtual void OnPlayStop() override;
		virtual void OnSimulationStart() override;
		virtual void OnSimulationStop() override;
	};

}

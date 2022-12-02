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

	class SystemBase : public ISystem
	{
	public:
		using ISystem::ISystem;

		virtual void OnCreate() {}
		virtual void OnUpdateEditor(DeltaTime dt) {}
		virtual void OnUpdateRuntime(DeltaTime dt) {}

		virtual void OnPlayStart() {}
		virtual void OnPlayStop() {}
		virtual void OnSimulationStart() {}
		virtual void OnSimulationStop() {}
	};

	class SceneObserverSystemBase
	{
	public:
		virtual ~SceneObserverSystemBase() = default;

		void SetScene(Scene* scene) { m_Scene = scene; }

		virtual Scope<SceneObserverSystemBase> Clone() = 0;

		virtual void OnBind() = 0;
		virtual void OnUnbind() = 0;
		virtual void OnUpdate(Scene& scene) {}

	protected:
		template<typename Component, auto FreeFunc>
		void BindOnComponentAdded()
		{
			m_Scene->on_construct<Component>().template connect<FreeFunc>();
		}

		template<typename Component, auto Func, typename Type>
		void BindOnComponentAdded(Type&& instance)
		{
			m_Scene->on_construct<Component>().template connect<Func>(std::forward<Type>(instance));
		}

		template<typename Component>
		void UnbindOnComponentAdded()
		{
			m_Scene->on_construct<Component>().disconnect();
		}

		template<typename Component, auto FreeFunc>
		void BindOnComponentUpdated()
		{
			m_Scene->on_update<Component>().template connect<FreeFunc>();
		}

		template<typename Component>
		void UnbindOnComponentUpdated()
		{
			m_Scene->on_update<Component>().disconnect();
		}

		template<typename Component, auto FreeFunc>
		void BindOnComponentDestroy()
		{
			m_Scene->on_destroy<Component>().template connect<FreeFunc>();
		}

		template<typename Component>
		void UnbindOnComponentDestroy()
		{
			m_Scene->on_destroy<Component>().disconnect();
		}

		template<typename... Matcher>
		[[nodiscard]] Scope<entt::observer> CreateObserver(entt::basic_collector<Matcher...>)
		{
			return CreateScope<entt::observer>(*m_Scene, entt::basic_collector<Matcher...>{});
		}

	private:
		Scene* m_Scene = nullptr;
	};

	class LevelObserverSystem : public SceneObserverSystemBase
	{
	public:
		virtual Scope<SceneObserverSystemBase> Clone() override { return CreateScope<LevelObserverSystem>(); }

		virtual void OnBind() override;
		virtual void OnUnbind() override;
		virtual void OnUpdate(Scene& scene) override;

	private:
		Scope<entt::observer> m_CameraObserver;
		Scope<entt::observer> m_BoundsObserver;
		Scope<entt::observer> m_PhysicsActorObserver;
		Scope<entt::observer> m_PhysicsShapeObserver;
		Scope<entt::observer> m_PhysicsControllerObserver;
	};

	class MeshPreviewObserverSystem : public SceneObserverSystemBase
	{
	public:
		virtual Scope<SceneObserverSystemBase> Clone() override { return CreateScope<MeshPreviewObserverSystem>(); }

		virtual void OnBind() override;
		virtual void OnUnbind() override;
		virtual void OnUpdate(Scene& scene) override;

	private:
		Scope<entt::observer> m_BoundsObserver;
	};

	class TexturePreviewObserverSystem : public SceneObserverSystemBase
	{
	public:
		virtual Scope<SceneObserverSystemBase> Clone() override { return CreateScope<TexturePreviewObserverSystem>(); }

		virtual void OnBind() override;
		virtual void OnUnbind() override;
	};

	class MaterialPreviewObserverSystem : public SceneObserverSystemBase
	{
	public:
		virtual Scope<SceneObserverSystemBase> Clone() override { return CreateScope<MaterialPreviewObserverSystem>(); }

		virtual void OnBind() override;
		virtual void OnUnbind() override;
		virtual void OnUpdate(Scene& scene) override;

	private:
		Scope<entt::observer> m_BoundsObserver;
	};

	class ParticleUpdateSystem : public SystemBase
	{
	public:
		using SystemBase::SystemBase;

		virtual void OnUpdateEditor(DeltaTime dt) override;
		virtual void OnUpdateRuntime(DeltaTime dt) override;
	};

	class ParticlePreviewUpdateSystem : public ParticleUpdateSystem
	{
	public:
		using ParticleUpdateSystem::ParticleUpdateSystem;

		virtual void OnUpdateEditor(DeltaTime dt) override;
		virtual void OnUpdateRuntime(DeltaTime dt) override;
	};

	class ScriptSystem : public SystemBase
	{
	public:
		using SystemBase::SystemBase;

		virtual void OnUpdateRuntime(DeltaTime dt) override;

		virtual void OnPlayStart() override;
		virtual void OnPlayStop() override;
	};

	class PhysicsSystem : public SystemBase
	{
	public:
		using SystemBase::SystemBase;

		virtual void OnUpdateRuntime(DeltaTime dt) override;

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

		virtual void OnUpdateRuntime(DeltaTime dt) override;

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

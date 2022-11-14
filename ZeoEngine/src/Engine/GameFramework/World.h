#pragma once

#include "Engine/Asset/Asset.h"
#include "Engine/Core/Core.h"
#include "Engine/Core/DeltaTime.h"
#include "Engine/GameFramework/Entity.h"

namespace ZeoEngine {

	class Scene;
	class SceneRenderer;
	class SystemBase;

	class WorldBase
	{
	public:
		explicit WorldBase(std::string worldName);
		WorldBase(const WorldBase&) = delete;
		WorldBase(WorldBase&&) = default;
		virtual ~WorldBase();

		WorldBase& operator=(const WorldBase&) = delete;
		WorldBase& operator=(WorldBase&&) = default;

		virtual void OnAttach();
		virtual void OnUpdate(DeltaTime dt);

		bool IsActive() const { return m_bActive; }
		void SetActive(bool bActive) { m_bActive = bActive; }
		const auto& GetSystems() const { return m_Systems; }

		void NewScene();
		Ref<Scene> GetActiveScene() const { return m_ActiveScene; }
		void SetActiveScene(Ref<Scene> scene);

		SceneRenderer* GetSceneRenderer() const { return m_SceneRenderer ? m_SceneRenderer.get() : nullptr; }

		const Ref<IAsset>& GetAsset() const { return m_Asset; }
		void SetAsset(Ref<IAsset> asset) { m_Asset = std::move(asset); }

		// To be defined in EditorPreviewWorldBase
		virtual Entity GetContextEntity() const { return {}; }

		virtual bool IsRuntime() const { return false; }
		virtual bool IsSimulation() const { return false; }

	protected:
		template<typename T, typename ... Args>
		void RegisterSystem(Args&& ... args)
		{
			static_assert(std::is_base_of_v<SystemBase, T>, "System type must be a SystemBase type!");

			auto system = CreateScope<T>(std::forward<Args>(args)...);
			system->OnCreate();
			m_Systems.emplace_back(std::move(system));
		}

	private:
		virtual Ref<Scene> CreateScene() = 0;
		virtual void PostSceneCreate(const Ref<Scene>& scene) {}
		virtual Ref<SceneRenderer> CreateSceneRenderer() = 0;

		void NewSceneRenderer();

		// For internal use purpose
		virtual bool IsRunning() { return false; }

	public:
		entt::sink<entt::sigh<void(Scene*, Scene*)>> m_OnActiveSceneChanged{ m_OnActiveSceneChangedDel };

	private:
		std::string m_WorldName;
		bool m_bActive = true;

		Ref<Scene> m_ActiveScene;
		Ref<SceneRenderer> m_SceneRenderer;
		std::vector<Scope<SystemBase>> m_Systems;

		Ref<IAsset> m_Asset;

		entt::sigh<void(Scene*, Scene*)> m_OnActiveSceneChangedDel;
	};

}

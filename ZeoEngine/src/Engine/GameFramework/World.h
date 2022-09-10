#pragma once

#include "Engine/Asset/Asset.h"
#include "Engine/Core/Core.h"
#include "Engine/Core/DeltaTime.h"

namespace ZeoEngine {

	class Scene;
	class SceneRenderer;
	class FrameBuffer;

	class WorldBase : public std::enable_shared_from_this<WorldBase>
	{
	protected:
		template<typename Derived>
		Ref<Derived> SharedFromBase()
		{
			return std::static_pointer_cast<Derived>(shared_from_this());
		}
		template<typename Derived>
		Ref<const Derived> SharedFromBase() const
		{
			return std::static_pointer_cast<const Derived>(shared_from_this());
		}

	private:
		using std::enable_shared_from_this<WorldBase>::shared_from_this;

	public:
		explicit WorldBase(std::string worldName);
		virtual ~WorldBase() = default;

		virtual void OnAttach();
		virtual void OnUpdate(DeltaTime dt);

		void NewScene();
		Ref<Scene>& GetActiveScene() { return m_ActiveScene; }
		template<typename T>
		Ref<T> GetActiveScene() const
		{
			return std::dynamic_pointer_cast<T>(m_ActiveScene);
		}
		void SetActiveScene(Ref<Scene> scene);

		Ref<SceneRenderer>& GetSceneRenderer() { return m_SceneRenderer; }

		const Ref<IAsset>& GetAsset() const { return m_Asset; }
		void SetAsset(Ref<IAsset> asset) { m_Asset = std::move(asset); }

	private:
		virtual Ref<Scene> CreateScene() = 0;
		virtual void PostSceneCreate(const Ref<Scene>& scene) {}
		virtual Ref<SceneRenderer> CreateSceneRenderer() = 0;

		void NewSceneRenderer();

	public:
		entt::sink<entt::sigh<void(const Ref<Scene>&)>> m_OnActiveSceneChanged{ m_OnActiveSceneChangedDel };
		entt::sink<entt::sigh<void(const Ref<FrameBuffer>&)>> m_PostSceneRender{ m_PostSceneRenderDel };

	private:
		std::string m_WorldName;

		Ref<Scene> m_ActiveScene;
		Ref<SceneRenderer> m_SceneRenderer;

		Ref<IAsset> m_Asset;

		entt::sigh<void(const Ref<Scene>&)> m_OnActiveSceneChangedDel;
		entt::sigh<void(const Ref<FrameBuffer>&)> m_PostSceneRenderDel;
	};

}

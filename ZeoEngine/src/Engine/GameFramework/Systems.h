#pragma once

#include "Engine/Core/DeltaTime.h"
#include "Engine/GameFramework/Scene.h"

class b2World;

namespace ZeoEngine {

	class SceneRenderer;

	template<typename... Component>
	inline constexpr entt::exclude_t<Component...> ExcludeComponents{};

	class ISystem
	{
	public:
		explicit ISystem(const Ref<Scene>& scene);

	protected:
		template<typename... Component, typename... Exclude, typename Func>
		void ForEachComponentView(Func&& func, entt::exclude_t<Exclude...> exclude = {})
		{
			m_Scene->m_Registry.view<Component...>(exclude).each(std::forward<Func>(func));
		}

		struct DefaultCompare
		{
			template<typename... Component>
			constexpr auto operator()(std::tuple<Component&...>, std::tuple<Component&...>) const
			{
				return true;
			}
		};

		template<typename... Owned, typename... Get, typename... Exclude, typename Func, typename CompareFunc = DefaultCompare>
		void ForEachComponentGroup(entt::get_t<Get...> get, Func&& func, CompareFunc compareFunc = CompareFunc{}, entt::exclude_t<Exclude...> exclude = {})
		{
			auto group = m_Scene->m_Registry.group<Owned..., Get...>(exclude);
			if constexpr (!std::is_same<CompareFunc, DefaultCompare>::value)
			{
				group.sort<Owned..., Get...>(std::move(compareFunc));
			}
			group.each(std::forward<Func>(func));
		}

	protected:
		Ref<Scene> m_Scene;
	};

	class SystemBase : public ISystem
	{
	public:
		explicit SystemBase(const Ref<Scene>& scene);

		virtual void OnCreate() {}
		void OnUpdate(DeltaTime dt);
		virtual void OnDestroy() {}

		void BindUpdateFuncToEditor();
		void BindUpdateFuncToRuntime();

		virtual void OnRuntimeStart() {}
		virtual void OnRuntimeStop() {}
	private:
		virtual void OnUpdateEditor(DeltaTime dt) {}
		virtual void OnUpdateRuntime(DeltaTime dt) {}

	private:
		entt::delegate<void(DeltaTime)> m_UpdateFuncDel;
	};

	class RenderSystemBase : public ISystem
	{
	public:
		RenderSystemBase(const Ref<Scene>& scene, const Ref<SceneRenderer>& sceneRenderer);
		
		virtual void OnRenderEditor() = 0;
		virtual void OnRenderRuntime() {}

		void UpdateScene(const Ref<Scene>& scene) { m_Scene = scene; }

		std::pair<Camera*, glm::mat4> GetActiveCamera();

	protected:
		Ref<SceneRenderer> m_SceneRenderer;
	};

	class RenderSystem : public RenderSystemBase
	{
	public:
		using RenderSystemBase::RenderSystemBase;

		virtual void OnRenderEditor();
		virtual void OnRenderRuntime();

	private:
		void RenderLights(bool bIsEditor);
		void RenderMeshes();
	};

	class RenderSystem2D : public RenderSystem
	{
	public:
		using RenderSystem::RenderSystem;

		virtual void OnRenderEditor() override;
		virtual void OnRenderRuntime() override;

	private:
		void OnRenderImpl();
	};

	class ParticlePreviewRenderSystem : public RenderSystemBase
	{
	public:
		using RenderSystemBase::RenderSystemBase;

		virtual void OnRenderEditor() override;
	};

	class MaterialPreviewRenderSystem : public RenderSystemBase
	{
	public:
		using RenderSystemBase::RenderSystemBase;

		virtual void OnRenderEditor() override;
	};

	class ParticleUpdateSystem : public SystemBase
	{
	public:
		using SystemBase::SystemBase;

		virtual void OnUpdateEditor(DeltaTime dt) override;
		virtual void OnUpdateRuntime(DeltaTime dt) override;
		virtual void OnDestroy() override;

	private:
		void OnUpdateImpl(DeltaTime dt);
	};

	class ParticlePreviewUpdateSystem : public SystemBase
	{
	public:
		using SystemBase::SystemBase;

		virtual void OnUpdateEditor(DeltaTime dt) override;
		virtual void OnUpdateRuntime(DeltaTime dt) override;
		virtual void OnDestroy() override;

	private:
		void OnUpdateImpl(DeltaTime dt);
	};

	class NativeScriptSystem : public SystemBase
	{
	public:
		using SystemBase::SystemBase;

		virtual void OnUpdateRuntime(DeltaTime dt) override;
		void OnEvent(Event& e);
	};

	class PhysicsSystem : public SystemBase
	{
	public:
		using SystemBase::SystemBase;

		virtual void OnUpdateRuntime(DeltaTime dt) override;

		virtual void OnRuntimeStart() override;
		virtual void OnRuntimeStop() override;
	};

	class PhysicsSystem2D : public PhysicsSystem
	{
	public:
		using PhysicsSystem::PhysicsSystem;

		virtual void OnUpdateRuntime(DeltaTime dt) override;

		virtual void OnRuntimeStart() override;
		virtual void OnRuntimeStop() override;

	private:
		b2World* m_PhysicsWorld = nullptr;
	};

}

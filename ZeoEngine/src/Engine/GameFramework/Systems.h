#pragma once

#include "Engine/Core/DeltaTime.h"
#include "Engine/GameFramework/Scene.h"

class b2World;

namespace ZeoEngine {

	template<typename... Component>
	inline constexpr entt::exclude_t<Component...> ExcludeComponents{};

	class ISystem
	{
	public:
		explicit ISystem(Scene* scene);

		virtual void OnCreate() {}
		virtual void OnUpdate(DeltaTime dt) = 0;
		virtual void OnDestroy() {}

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
		Scene* m_Scene = nullptr;
	};

	class RenderSystem : public ISystem
	{
	public:
		using ISystem::ISystem;
		
		virtual void OnUpdate(DeltaTime dt) override;
		virtual void OnRenderEditor(const EditorCamera& camera);
		virtual void OnRenderRuntime();
		virtual void OnDestroy() override;

	private:
		void OnRender(const glm::vec3& cameraPosition);
	};

	class RenderSystem2D : public RenderSystem
	{
	public:
		using RenderSystem::RenderSystem;

		virtual void OnRenderEditor(const EditorCamera& camera) override;
		virtual void OnRenderRuntime() override;

	private:
		void OnRender();
	};

	class ParticlePreviewRenderSystem : public ISystem
	{
	public:
		using ISystem::ISystem;

		virtual void OnUpdate(DeltaTime dt) override;
		void OnRender(const EditorCamera& camera);
		virtual void OnDestroy() override;
	};

	class MaterialPreviewRenderSystem : public ISystem
	{
	public:
		using ISystem::ISystem;

		virtual void OnUpdate(DeltaTime dt) override {}
		void OnRender(const EditorCamera& camera);
	};

	class NativeScriptSystem : public ISystem
	{
	public:
		using ISystem::ISystem;

		virtual void OnUpdate(DeltaTime dt) override;
		void OnEvent(Event& e);
	};

	class PhysicsSystem : public ISystem
	{
	public:
		using ISystem::ISystem;

		virtual void OnUpdate(DeltaTime dt) override;

		virtual void OnRuntimeStart();
		virtual void OnRuntimeStop();
	};

	class PhysicsSystem2D : public PhysicsSystem
	{
	public:
		using PhysicsSystem::PhysicsSystem;

		virtual void OnUpdate(DeltaTime dt) override;

		virtual void OnRuntimeStart() override;
		virtual void OnRuntimeStop() override;

	private:
		b2World* m_PhysicsWorld = nullptr;
	};

}

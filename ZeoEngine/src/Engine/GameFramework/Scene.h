#pragma once

#include <entt.hpp>
#define DEBUG_DRAW_EXPLICIT_CONTEXT
#include <debug_draw.hpp>

#include "Engine/Asset/Asset.h"
#include "Engine/Core/Core.h"
#include "Engine/Events/Event.h"

namespace ZeoEngine {

	class PhysXScene;

	/** A scene context which shares among copied scenes. */
	struct SceneContext
	{
		dd::ContextHandle DebugDrawContext = nullptr;
	};

	template<typename... Component>
	inline constexpr entt::get_t<Component...> IncludeComponents{};

	template<typename... Component>
	inline constexpr entt::exclude_t<Component...> ExcludeComponents{};

	struct SceneSpec
	{
		Scope<class SceneObserverSystemBase> SceneObserverSystem;
		bool bIsPhysicalScene = false;

		SceneSpec Clone();
	};

	class Scene final : public entt::registry, public std::enable_shared_from_this<Scene>
	{
		friend class Entity;
		friend class SceneObserverSystemBase;

	public:
		explicit Scene(SceneSpec spec);
		~Scene();

		void OnUpdate();

		// We cannot copy Scene through copy constructor as it is a deleted function
		// So we construct a new scene and do the copy ourselves
		Ref<Scene> Copy();

		SceneContext* GetContextShared() const { return m_ContextShared.get(); }

		void CreatePhysicsScene();
		/** PhyXScene only exists at runtime when bIsPhysicalScene is true. */
		PhysXScene* GetPhysicsScene();
		void DestroyPhysicsScene();

		Entity CreateEntity(const std::string& name = "Entity", const Vec3& translation = Vec3(0.0f));
		Entity CreateEntityWithUUID(UUID uuid, const std::string& name = "Entity", const Vec3& translation = Vec3(0.0f));
		Entity DuplicateEntity(Entity entity);
		void DestroyEntity(Entity entity);
		Entity GetEntityByUUID(UUID uuid) const;
		Entity GetEntityByName(std::string_view name);
		SizeT GetEntityCount() const { return alive(); }

		Entity GetMainCameraEntity();

		template<typename T, typename... Args>
		T& AddContext(Args&&... args)
		{
			ZE_CORE_ASSERT(!HasContext<T>(), "Scene already has context!");
			return ctx().emplace<T>(std::forward<Args>(args)...);
		}

		template<typename T>
		bool RemoveContext()
		{
			return ctx().erase<T>();
		}

		template<typename T>
		T& GetContext()
		{
			ZE_CORE_ASSERT(HasContext<T>(), "Scene does not have context!");
			return ctx().at<T>();
		}

		template<typename T>
		bool HasContext() const
		{
			return ctx().contains<T>();
		}

		template<typename... Component, typename... Exclude, typename Func>
		void ForEachComponentView(Func&& func, entt::exclude_t<Exclude...> exclude = {}) const
		{
			view<Component...>(exclude).each(std::forward<Func>(func));
		}
		template<typename... Component, typename... Exclude, typename Func>
		void ForEachComponentView(Func&& func, entt::exclude_t<Exclude...> exclude = {})
		{
			view<Component...>(exclude).each(std::forward<Func>(func));
		}

		struct DefaultCompare
		{
			template<typename... Component>
			constexpr auto operator()(std::tuple<Component&...>, std::tuple<Component&...>) const { return true; }
		};

		// TODO:
		template<typename... Owned, typename... Include, typename... Exclude, typename Func, typename CompareFunc = DefaultCompare>
		void ForEachComponentGroup(entt::get_t<Include...> include, Func&& func, entt::exclude_t<Exclude...> exclude = {}, CompareFunc compareFunc = CompareFunc{})
		{
			auto compGroup = group<Owned...>(include, exclude);
			if constexpr (!std::is_same_v<CompareFunc, DefaultCompare>)
			{
				compGroup.sort<Owned..., Include...>(std::move(compareFunc));
			}
			compGroup.each(std::forward<Func>(func));
		}

		template<typename... Owned, typename... Exclude, typename Func, typename CompareFunc = DefaultCompare>
		void ForEachComponentGroup(Func&& func, entt::exclude_t<Exclude...> exclude = {}, CompareFunc compareFunc = CompareFunc{})
		{
			auto compGroup = group<Owned...>(exclude);
			if constexpr (!std::is_same_v<CompareFunc, DefaultCompare>)
			{
				compGroup.sort<Owned...>(std::move(compareFunc));
			}
			compGroup.each(std::forward<Func>(func));
		}

	private:
		// TODO: Hide more parent methods
		using entt::registry::ctx;
		using entt::registry::view;
		using entt::registry::group;
		using entt::registry::create;
		using entt::registry::destroy;
		using entt::registry::valid;
		using entt::registry::emplace;
		using entt::registry::erase;
		using entt::registry::remove;
		using entt::registry::get;
		using entt::registry::all_of;
		using entt::registry::patch;
		using entt::registry::on_construct;
		using entt::registry::on_update;
		using entt::registry::on_destroy;

		void SortEntities();

	private:
		SceneSpec m_Spec;
		Ref<SceneContext> m_ContextShared;

		std::unordered_map<UUID, Entity> m_Entities;
		U32 m_CurrentEntityIndex = 0;

	};

	class Level : public AssetBase<Level>
	{
	public:
		static constexpr const char* GetTemplatePath() { return "Engine/levels/NewLevel.zasset"; }

		Scene* GetScene() const { return m_Scene; }
		void SetScene(Scene* scene) { m_Scene = scene; }

	private:
		Scene* m_Scene = nullptr;
	};

}

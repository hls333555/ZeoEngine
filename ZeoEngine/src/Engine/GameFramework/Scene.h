#pragma once

#include <entt.hpp>
#define DEBUG_DRAW_EXPLICIT_CONTEXT
#include <debug_draw.hpp>

#include "Engine/Asset/Asset.h"
#include "Engine/Core/Core.h"
#include "Engine/Core/DeltaTime.h"
#include "Engine/Events/Event.h"

namespace ZeoEngine {

	class EditorCamera;
	class SystemBase;
	class WorldBase;

	/** A scene context which shares among copied scenes. */
	struct SceneContext
	{
		dd::ContextHandle DebugDrawContext = nullptr;
	};

	template<typename... Component>
	inline constexpr entt::exclude_t<Component...> ExcludeComponents{};

	class Scene : public std::enable_shared_from_this<Scene>
	{
		friend class Entity;
		friend class LevelViewPanel;

	public:
		Scene() = default;
		virtual ~Scene();

		virtual void OnAttach(const Ref<WorldBase>& world) {}
		void OnUpdate(DeltaTime dt) const;
		virtual void OnEvent(Event& e) {}

		/** Copy function which processes member copy. */
		virtual void Copy(const Ref<Scene>& other);

		// We cannot copy Scene through copy constructor as it is a deleted function
		// So we construct a new scene and call our "copy" function instead
		template<typename T, typename ... Args>
		Ref<T> Copy(Args&& ... args)
		{
			Ref<T> newScene = CreateRef<T>(std::forward<Args>(args)...);
			newScene->Copy(shared_from_this());
			return newScene;
		}

		const auto& GetSystems() const { return m_Systems; }
		const auto& GetContext() const { return m_Context; }

		Entity CreateEntity(const std::string& name = "Entity", const Vec3& translation = Vec3(0.0f));
		Entity CreateEntityWithUUID(UUID uuid, const std::string& name = "Entity", const Vec3& translation = Vec3(0.0f));
		Entity DuplicateEntity(Entity entity);
		void DestroyEntity(Entity entity);
		Entity GetEntityByUUID(UUID uuid);
		SizeT GetEntityCount() const { return m_Registry.alive(); }

		/** Called after all data have been loaded. */
		virtual void PostLoad() {}

		template<typename... Component, typename... Exclude, typename Func>
		void ForEachComponentView(Func&& func, entt::exclude_t<Exclude...> exclude = {})
		{
			m_Registry.view<Component...>(exclude).each(std::forward<Func>(func));
		}

		struct DefaultCompare
		{
			template<typename... Component>
			constexpr auto operator()(std::tuple<Component&...>, std::tuple<Component&...>) const { return true; }
		};

		template<typename... Owned, typename... Get, typename... Exclude, typename Func, typename CompareFunc = DefaultCompare>
		void ForEachComponentGroup(entt::get_t<Get...> get, Func&& func, CompareFunc compareFunc = CompareFunc{}, entt::exclude_t<Exclude...> exclude = {})
		{
			auto group = m_Registry.group<Owned..., Get...>(exclude);
			if constexpr (!std::is_same<CompareFunc, DefaultCompare>::value)
			{
				group.sort<Owned..., Get...>(std::move(compareFunc));
			}
			group.each(std::forward<Func>(func));
		}

		template<typename Component, auto Candidate, typename Type>
		void BindOnComponentConstruct(Type&& valueOrInstance)
		{
			m_Registry.on_construct<Component>().connect<Candidate>(std::forward<Type>(valueOrInstance));
		}

		template<typename Component, auto Candidate, typename Type>
		void UnbindOnComponentConstruct(Type&& valueOrInstance)
		{
			m_Registry.on_construct<Component>().disconnect<Candidate>(std::forward<Type>(valueOrInstance));
		}

	protected:
		template<typename T, typename ... Args>
		void RegisterSystem(Args&& ... args)
		{
			static_assert(std::is_base_of_v<SystemBase, T>, "System type must be a SystemBase type!");

			Ref<T> system = CreateRef<T>(std::forward<Args>(args)...);
			system->OnCreate();
			m_Systems.emplace_back(std::move(system));
		}

	private:
		void SortEntities();

	public:
		entt::sink<entt::sigh<void(const Ref<Scene>&)>> m_OnSceneCopied{ m_OnSceneCopiedDel };
	private:
		entt::registry m_Registry;

		std::unordered_map<UUID, Entity> m_Entities;
		/** Systems are shared among copied scenes. */
		std::vector<Ref<SystemBase>> m_Systems;

		Ref<SceneContext> m_Context = CreateRef<SceneContext>();

		U32 m_CurrentEntityIndex = 0;
		entt::sigh<void(const Ref<Scene>&)> m_OnSceneCopiedDel;
	};

	class Level : public AssetBase<Level>
	{
	public:
		static constexpr const char* GetTemplatePath() { return "Engine/levels/NewLevel.zasset"; }

		const Ref<Scene>& GetScene() const { return m_Scene; }
		void SetScene(const Ref<Scene>& scene) { m_Scene = scene; }

	private:
		Ref<Scene> m_Scene;
	};

}

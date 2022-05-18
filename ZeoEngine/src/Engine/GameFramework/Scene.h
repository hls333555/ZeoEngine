#pragma once

#include <entt.hpp>

#include "Engine/Core/Asset.h"
#include "Engine/Core/Core.h"
#include "Engine/Core/DeltaTime.h"
#include "Engine/Events/Event.h"
#include "Engine/Core/AssetLibrary.h"
#include "Engine/GameFramework/Components.h"

namespace ZeoEngine {

	 class EditorCamera;
	 class SystemBase;

	class Scene : public std::enable_shared_from_this<Scene>
	{
		friend class Entity;
		friend class SceneSerializer;
		friend class EditorViewPanelBase;
		friend class SceneOutlinePanel;
		friend class ISystem;

	public:
		Scene() = default;
		virtual ~Scene();

		virtual void OnAttach() {}
		void OnUpdate(DeltaTime dt) const;
		virtual void OnEvent(Event& e) {}

		const auto& GetSystems() const { return m_Systems; }

		template<typename T, typename ... Args>
		Ref<T> Copy(Args&& ... args)
		{
			Ref<T> newScene = CreateRef<T>(std::forward<Args>(args)...);
			newScene->m_Systems = m_Systems;
			m_Registry.view<CoreComponent>().each([this, &newScene](auto entityId, auto& coreComp)
			{
				Entity entity{ entityId, shared_from_this() };
				// Clone a new "empty" entity
				auto newEntity = newScene->CreateEntityWithUUID(entity.GetUUID(), entity.GetName());
				// Copy components to that entity
				newEntity.CopyAllComponents(entity);
			});
			m_OnSceneCopiedDel.publish(newScene);
			return newScene;
		}

		Entity CreateEntity(const std::string& name = "Entity", const glm::vec3& translation = glm::vec3(0.0f));
		Entity CreateEntityWithUUID(UUID uuid, const std::string& name = "Entity", const glm::vec3& translation = glm::vec3(0.0f));
		Entity DuplicateEntity(Entity entity);
		void DestroyEntity(Entity entity);

		size_t GetEntityCount() const { return m_Registry.alive(); }

		/** Only implemented for the level editor. */
		virtual Entity GetSelectedEntity() const;

		/** Called after all data have been loaded. */
		virtual void PostLoad() {}

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
	protected:
		entt::registry m_Registry;
	private:
		std::vector<Ref<SystemBase>> m_Systems;
		uint32_t m_CurrentEntityIndex = 0;
		entt::sigh<void(const Ref<Scene>&)> m_OnSceneCopiedDel;
	};

	class Level : public AssetBase<Level>
	{
	public:
		explicit Level(std::string ID, const Ref<Scene>& scene)
			: AssetBase(std::move(ID)), m_Scene(scene) {}

		static Ref<Level> Create(std::string ID, const Ref<Scene>& scene);

		const Ref<Scene>& GetScene() const { return m_Scene; }

		virtual void Serialize(const std::string& path) override;
		virtual void Deserialize() override;

	private:
		Ref<Scene> m_Scene;
	};

	REGISTER_ASSET(Level,
	Ref<Level> operator()(std::string ID, bool bIsReload, const Ref<Scene>& scene) const
	{
		return Level::Create(std::move(ID), scene);
	},
	static AssetHandle<Level> GetDefaultEmptyLevel(const Ref<Scene>& scene)
	{
		return Get().LoadAsset("ZID_DefaultEmptyLevel", scene);
	})

}

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
		void OnUpdate(DeltaTime dt);
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

			Ref<T> system = CreateScope<T>(std::forward<Args>(args)...);
			system->OnCreate();
			m_Systems.emplace_back(std::move(system));
		}

	private:
		void SortEntities();

	public:
		entt::sink<void(const Ref<Scene>&)> m_OnSceneCopied{ m_OnSceneCopiedDel };
	protected:
		entt::registry m_Registry;
	private:
		// TODO: Copy performance?
		std::vector<Ref<SystemBase>> m_Systems;
		uint32_t m_CurrentEntityIndex = 0;
		entt::sigh<void(const Ref<Scene>&)> m_OnSceneCopiedDel;
	};

	class SceneAsset : public AssetBase<SceneAsset>
	{
	private:
		explicit SceneAsset(const std::string& path);

	public:
		static Ref<SceneAsset> Create(const std::string& path = "");

		const Ref<Scene>& GetScene() const { return m_Scene; }
		/** Update scene referenece. */
		void UpdateScene(const Ref<Scene>& scene) { m_Scene = scene; }
		/** Clear scene referenece. */
		void ClearScene() { m_Scene.reset(); }

		virtual void Serialize(const std::string& path) override;
		virtual void Deserialize() override;

		virtual void Reload(bool bIsCreate) override;

	private:
		Ref<Scene> m_Scene;
	};

	struct SceneAssetLoader final
	{
		using result_type = Ref<SceneAsset>;

		Ref<SceneAsset> operator()(const std::string& path) const
		{
			return SceneAsset::Create(path);
		}
	};

	class SceneAssetLibrary : public AssetLibrary<SceneAssetLibrary, SceneAsset, SceneAssetLoader>{};

}

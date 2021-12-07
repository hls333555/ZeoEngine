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

	class Scene
	{
		friend class Entity;
		friend class SceneSerializer;
		friend class EditorViewPanelBase;
		friend class SceneOutlinePanel;
		friend class ISystem;

	public:
		Scene() = default;
		virtual ~Scene() = default;

		virtual void OnUpdate(DeltaTime dt) {}
		virtual void OnRender(const EditorCamera& camera) {}
		virtual void OnEvent(Event& e) {}

		template<typename T, typename ... Args>
		Ref<T> Copy(Args&& ... args)
		{
			Ref<T> newScene = CreateRef<T>(std::forward<Args>(args)...);
			m_Registry.view<CoreComponent>().each([this, &newScene](auto entityId, auto& coreComp)
			{
				Entity entity{ entityId, this };
				// Clone a new "empty" entity
				auto newEntity = newScene->CreateEntityWithUUID(entity.GetUUID(), entity.GetName());
				// Copy components to that entity
				newEntity.CopyAllComponents(entity);
			});
			return newScene;
		}

		Entity CreateEntity(const std::string& name = "Entity");
		Entity CreateEntityWithUUID(UUID uuid, const std::string& name = "Entity");
		Entity DuplicateEntity(Entity entity);
		void DestroyEntity(Entity entity);

		size_t GetEntityCount() const { return m_Registry.alive(); }

		/** Only implemented for the level editor. */
		virtual Entity GetSelectedEntity() const;

		/** Called after all data have been loaded. */
		virtual void PostLoad() {}

	private:
		void SortEntities();

	protected:
		entt::registry m_Registry;

	private:
		uint32_t m_CurrentEntityIndex = 0;
	};

	class SceneAsset : public AssetBase<SceneAsset>
	{
	private:
		explicit SceneAsset(const std::string& path);

	public:
		static AssetHandle<SceneAsset> Create(const std::string& path = "");

		const Ref<Scene>& GetScene() const { return m_Scene; }
		/** Update scene referenece. */
		void UpdateScene(const Ref<Scene>& scene) { m_Scene = scene; }
		/** Clear scene referenece. */
		void ClearScene() { m_Scene.reset(); }

		virtual void Serialize(const std::string& path) override;
		virtual void Deserialize() override;

		virtual void Reload() override;

	private:
		Ref<Scene> m_Scene;
	};

	struct SceneAssetLoader final : AssetLoader<SceneAssetLoader, SceneAsset>
	{
		AssetHandle<SceneAsset> load(const std::string& path) const
		{
			return SceneAsset::Create(path);
		}
	};

	class SceneAssetLibrary : public AssetLibrary<SceneAssetLibrary, SceneAsset, SceneAssetLoader>{};

}

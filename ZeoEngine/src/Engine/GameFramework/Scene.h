#pragma once

#include <entt.hpp>

#include "Engine/Core/Asset.h"
#include "Engine/Core/Core.h"
#include "Engine/Core/DeltaTime.h"
#include "Engine/Events/Event.h"
#include "Engine/Core/AssetLibrary.h"

namespace ZeoEngine {

	 class EditorCamera;

	class Scene
	{
		friend class Entity;
		friend class SceneSerializer;
		friend class SceneViewportPanel;
		friend class SceneOutlinePanel;
		friend class ISystem;
		friend class RenderSystem;

	public:
		Scene() = default;
		virtual ~Scene() = default;

		virtual void OnUpdate(DeltaTime dt) {}
		virtual void OnRender(const EditorCamera& camera) {}
		virtual void OnEvent(Event& e) {}

		/**
		 * Create an entity with default components.
		 * 
		 * @param bIsInternal - If true, this entity will not show in SceneOutlinePanel.
		 */
		Entity CreateEntity(const std::string& name = "Entity", bool bIsInternal = false);
		void DestroyEntity(Entity entity);

		/** Called after all data have been loaded. */
		virtual void PostLoad() {}

	private:
		/** Create an entity with no default components. */
		Entity CreateEmptyEntity();

		void SortEntities();

	protected:
		entt::registry m_Registry;

	private:
		uint32_t m_EntityCount = 0;
	};

	class SceneAsset : public AssetImpl<SceneAsset>
	{
	private:
		explicit SceneAsset(const std::string& path);

	public:
		static Ref<SceneAsset> Create(const std::string& path);

		/** Update scene reference and deserialize scene data. */
		void UpdateScene(const Ref<Scene>& scene) { m_Scene = scene; Deserialize(); }
		/** Clear scene referenece. */
		void ClearScene() { m_Scene.reset(); }

		virtual void Serialize(const std::string& path) override;
		virtual void Deserialize() override;

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

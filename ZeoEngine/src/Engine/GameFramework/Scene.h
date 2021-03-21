#pragma once

#include <entt.hpp>

#include "Engine/Core/DeltaTime.h"
#include "Engine/Events/Event.h"

namespace ZeoEngine {

	 class EditorCamera;

	class Scene
	{
		friend class Entity;
		friend class SceneSerializer;
		friend class SceneViewportPanel;
		friend class SceneOutlinePanel;
		friend class DataInspectorPanel;
		friend class DataInspector;

	public:
		Scene() = default;
		virtual ~Scene() = default;

		const std::string& GetName() const { return m_Name; }
		const std::string& GetPath() const { return m_Path; }
		void SetPath(const std::string& path);

		/**
		 * Create an entity with default components.
		 * @param bIsInternal - If true, this entity will not show in the SceneOutlinePanel.
		 */
		Entity CreateEntity(const std::string& name = "Entity", bool bIsInternal = false);

	private:
		/** Create an entity with no default components. */
		Entity CreateEmptyEntity();
	public:
		void DestroyEntity(Entity entity);

		void SortEntities();

		virtual void OnUpdate(DeltaTime dt) {}
		virtual void OnRender(const EditorCamera& camera) {}
		virtual void OnEvent(Event& e) {}

		/** Called after scene has been deserialized. */
		virtual void OnDeserialized() {}

	protected:
		// Should be called in destructor
		virtual void OnClenup() {}

	protected:
		entt::registry m_Registry;

	private:
		std::string m_Name{"Untitled"};
		std::string m_Path;

		uint32_t m_EntityCount;
	};

}

#pragma once

#include <entt.hpp>

#include "Engine/Core/DeltaTime.h"
#include "Engine/Events/Event.h"

namespace ZeoEngine {

	class Scene
	{
		friend class Entity;
		friend class SceneSerializer;
		friend class SceneViewportPanel;
		friend class SceneOutlinePanel;
		friend class DataInspectorPanel;
		friend class DataInspector;

	public:
		Scene();
		~Scene();

		/**
		 * Create an entity with default components.
		 * @param bIsInternal - If true, this entity will not show in the SceneOutlinePanel.
		 */
		Entity CreateEntity(const std::string& name = "Entity", bool bIsInternal = false);

		/** Create an entity with no default components. */
		Entity CreateEmptyEntity();
		void DestroyEntity(Entity entity);

		void SortEntities();

		void OnUpdate(DeltaTime dt);
		void OnRender();
		void OnEvent(Event& e);

		const std::string& GetName() const { return m_Name; }
		const std::string& GetPath() const { return m_Path; }
		void SetPath(const std::string& path);

	private:
		entt::registry m_Registry;
		uint32_t m_EntityCount = 0;
		std::string m_Name{"Untitled"};
		std::string m_Path;
	};

}

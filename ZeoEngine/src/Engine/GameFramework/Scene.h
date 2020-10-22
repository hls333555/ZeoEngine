#pragma once

#include <entt.hpp>

#include "Engine/Core/DeltaTime.h"
#include "Engine/Events/Event.h"

namespace ZeoEngine {

	class Scene
	{
		friend class Entity;
		friend class SceneViewportPanel;
		friend class SceneOutlinePanel;
		friend class DataInspectorPanel;
		friend class DataInspector;

	public:
		Scene();
		~Scene();

		/** Create an entity with default components. If bIsInternal is true, this entity will not show in the SceneOutlinePanel. */
		Entity CreateEntity(const std::string& name = std::string(), bool bIsInternal = false);
		/** Create an entity with no default components. */
		Entity CreateEmptyEntity();
		void DestroyEntity(Entity entity);

		void OnUpdate(DeltaTime dt);
		void OnRender();
		void OnEvent(Event& e);

	private:
		entt::registry m_Registry;
	};

}

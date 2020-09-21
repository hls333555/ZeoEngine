#pragma once

#include "entt.hpp"

#include "Engine/Core/DeltaTime.h"

namespace ZeoEngine {

	class Scene
	{
		friend class Entity;
		friend class SceneViewportPanel;
		friend class SceneOutlinePanel;

	public:
		Scene();
		~Scene();

		/** If bForInternalUse is true, this entity will not show in the SceneOutlinePanel. */
		Entity CreateEntity(const std::string& name = std::string(), bool bForInternalUse = false);

		void OnUpdate(DeltaTime dt);
		void OnRender();

	private:
		entt::registry m_Registry;
	};

}

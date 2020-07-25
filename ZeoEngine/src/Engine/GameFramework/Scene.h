#pragma once

#include "entt.hpp"

#include "Engine/Core/DeltaTime.h"

namespace ZeoEngine {

	class Scene
	{
		friend class Entity;

	public:
		Scene();
		~Scene();

		Entity CreateEntity(const std::string& name = std::string());

		void OnUpdate(DeltaTime dt);

	private:
		entt::registry m_Registry;
	};

}

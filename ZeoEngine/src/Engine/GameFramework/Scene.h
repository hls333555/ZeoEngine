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

	public:
		Scene() = default;
		virtual ~Scene() = default;

		virtual void OnUpdate(DeltaTime dt) {}
		virtual void OnRender(const EditorCamera& camera) {}
		virtual void OnEvent(Event& e) {}

		const std::string& GetName() const { return m_Name; }
		const std::string& GetPath() const { return m_Path; }
		void SetPath(const std::string& path);

		/**
		 * Create an entity with default components.
		 * @param bIsInternal - If true, this entity will not show in SceneOutlinePanel.
		 */
		Entity CreateEntity(const std::string& name = "Entity", bool bIsInternal = false);
		void DestroyEntity(Entity entity);

		/** Called after scene has been deserialized. */
		virtual void OnDeserialized() {}

	protected:
		// Should be called manually in destructor
		virtual void OnClenup() {}

	private:
		/** Create an entity with no default components. */
		Entity CreateEmptyEntity();

		void SortEntities();

	protected:
		entt::registry m_Registry;

	private:
		std::string m_Name{ "Untitled" };
		std::string m_Path;

		uint32_t m_EntityCount;
	};

}

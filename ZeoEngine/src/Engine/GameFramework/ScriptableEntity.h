#pragma once

#include "Engine/GameFramework/Entity.h"

namespace ZeoEngine {

	class ScriptableEntity
	{
		friend class Scene;

	public:
		virtual ~ScriptableEntity() = default;

		template<typename T>
		T& GetComponent()
		{
			return m_Entity.GetComponent<T>();
		}

		virtual void OnCreate() {}
		virtual void OnDestroy() {}
		virtual void OnUpdate(DeltaTime dt) {}

	private:
		Entity m_Entity;

	};

}

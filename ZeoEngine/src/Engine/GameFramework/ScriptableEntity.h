#pragma once

#include "Engine/GameFramework/Entity.h"

namespace ZeoEngine {

	class ScriptableEntity
	{
		friend class Scene;

	public:
		template<typename T>
		T& GetComponent()
		{
			return m_Entity.GetComponent<T>();
		}

	private:
		Entity m_Entity;

	};

}

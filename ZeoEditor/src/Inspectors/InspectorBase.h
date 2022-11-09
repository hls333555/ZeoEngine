#pragma once

#include "Engine/GameFramework/Entity.h"

namespace ZeoEngine {

	class InspectorBase
	{
	public:
		virtual ~InspectorBase() = default;

		virtual void Draw(Entity entity) = 0;
	};

}

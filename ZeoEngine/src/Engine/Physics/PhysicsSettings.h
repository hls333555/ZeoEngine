#pragma once

#include "Engine/Core/EngineTypes.h"
#include "Engine/Physics/PhysXDebugger.h"

namespace ZeoEngine {

	struct PhysicsSettings
	{
		float FixedDeltaTime = 1.0f / 60.0f;
		Vec3 Gravity{ 0.0f, -9.81f, 0.0f };

#ifdef ZE_DEBUG
		bool bDebugOnPlay = true;
		PhysXDebugType DebugType = PhysXDebugType::LiveDebug;
#endif
	};
	
}

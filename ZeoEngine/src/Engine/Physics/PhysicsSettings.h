#pragma once

#include "Engine/Core/EngineTypes.h"
#include "Engine/Physics/PhysicsTypes.h"
#ifdef ZE_DEBUG
#include "Engine/Physics/PhysXDebugger.h"
#endif

namespace ZeoEngine {

	// TODO: ProjectSettings
	struct PhysicsSettings
	{
		float FixedDeltaTime = 1.0f / 60.0f;
		Vec3 Gravity{ 0.0f, -9.81f, 0.0f };
		BroadphaseType BroadphaseAlgorithm = BroadphaseType::AutomaticBoxPrune;
		Vec3 WorldBoundsMin = Vec3{ -100.0f }; // Only valid when BroadphaseAlgorithm is MBP
		Vec3 WorldBoundsMax = Vec3{ 100.0f }; // Only valid when BroadphaseAlgorithm is MBP
		U32 WorldBoundsSubdivisions = 2; // Only valid when BroadphaseAlgorithm is MBP

#ifdef ZE_DEBUG
		bool bDebugOnPlay = true;
		PhysXDebugType DebugType = PhysXDebugType::LiveDebug;
#endif
	};
	
}

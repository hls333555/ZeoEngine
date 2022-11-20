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
		Vec3 WorldBoundsCenter = Vec3{ 0.0f }; // Only valid when BroadphaseAlgorithm is MBP
		Vec3 WorldBoundsExtent = Vec3{ 100.0f }; // Only valid when BroadphaseAlgorithm is MBP
		U32 WorldBoundsSubdivisions = 2; // Only valid when BroadphaseAlgorithm is MBP

		FrictionType FrictionModel = FrictionType::Patch;

		SolverType SolverModel = SolverType::ProjectedGaussSeidel;
		U32 SolverPositionIterations = 8;
		U32 SolverVelocityIterations = 2;

		bool bEnableEnhancedDeterminism = false;

#ifdef ZE_DEBUG
		bool bDebugOnPlay = true;
		PhysXDebugType DebugType = PhysXDebugType::LiveDebug;
#endif
	};
	
}

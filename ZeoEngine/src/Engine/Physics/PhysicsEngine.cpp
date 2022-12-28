#include "ZEpch.h"
#include "Engine/Physics/PhysicsEngine.h"

#include "Engine/Core/Console.h"
#include "Engine/Core/ConsoleVariables.h"
#include "Engine/Physics/PhysXEngine.h"
#include "Engine/Physics/CollisionLayer.h"

namespace ZeoEngine {

	PhysicsSettings PhysicsEngine::s_Settings;

	void PhysicsEngine::Init()
	{
		PhysXEngine::Init();

		CollisionLayerManager::AddDefaultLayers();
		CollisionLayerManager::AddDefaultGroups();

		Console::Get().RegisterVariable(CVAR_PHYSICS_DRAWCOLLIDERS, 0, "0: Hide, 1: Show", CommandType::RuntimeOnly);
		Console::Get().RegisterVariable(CVAR_PHYSICS_DRAWWORLDBOUNDS, 0, "0: Hide, 1: Show. Note that world bounds are only useful if BroadphaseAlgorithm is set to MultiBoxPrune.");

		ZE_CORE_TRACE("Physics engine intialized");
	}

	void PhysicsEngine::Shutdown()
	{
		PhysXEngine::Shutdown();
	}

}

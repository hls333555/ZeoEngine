#include "ZEpch.h"
#include "Engine/Physics/PhysicsEngine.h"

#include "Engine/Core/Console.h"
#include "Engine/Core/ConsoleVariables.h"
#include "Engine/Physics/PhysXEngine.h"

namespace ZeoEngine {

	PhysicsSettings PhysicsEngine::s_Settings;

	void PhysicsEngine::Init()
	{
		PhysXEngine::Init();

		Console::Get().RegisterVariable(CVAR_PHYSICS_DRAWCOLLIDERS, 0, "0: Disable, 1: Enable");

		ZE_CORE_TRACE("Physics engine intialized");
	}

	void PhysicsEngine::Shutdown()
	{
		PhysXEngine::Shutdown();
	}

}

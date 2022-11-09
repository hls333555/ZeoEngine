#include "ZEpch.h"
#include "Engine/Physics/PhysicsEngine.h"

#include "Engine/Physics/PhysXEngine.h"

namespace ZeoEngine {

	PhysicsSettings PhysicsEngine::s_Settings;

	void PhysicsEngine::Init()
	{
		PhysXEngine::Init();

		ZE_CORE_TRACE("Physics engine intialized");
	}

	void PhysicsEngine::Shutdown()
	{
		PhysXEngine::Shutdown();
	}

}

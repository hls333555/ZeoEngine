#include "ZEpch.h"
#include "Engine/Physics/PhysicsEngine.h"

#include "Engine/Physics/PhysXEngine.h"

namespace ZeoEngine {

	void PhysicsEngine::Init()
	{
		PhysXEngine::Init();
	}

	void PhysicsEngine::Shutdown()
	{
		PhysXEngine::Shutdown();
	}

}

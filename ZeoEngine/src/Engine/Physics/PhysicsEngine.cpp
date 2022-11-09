#include "ZEpch.h"
#include "Engine/Physics/PhysicsEngine.h"

#include <PxPhysicsAPI.h>

namespace ZeoEngine {

	struct PhysXData
	{
		
	};

	static PhysXData* s_Data = nullptr;

	void PhysicsEngine::Init()
	{
		s_Data = new PhysXData();
	}

	void PhysicsEngine::Shutdown()
	{
		delete s_Data;
		s_Data = nullptr;
	}

}

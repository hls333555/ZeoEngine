#include "ZEpch.h"
#include "Engine/Physics/PhysXCookingFactory.h"

#include "Engine/Physics/PhysXEngine.h"

namespace ZeoEngine {

	struct PhysXCookingData
	{
		physx::PxCooking* Cooking;
	};

	static PhysXCookingData* s_Data = nullptr;

	void PhysXCookingFactory::Init()
	{
		s_Data = new PhysXCookingData();

		physx::PxCookingParams cookingParams(PhysXEngine::GetTolerancesScale());
		s_Data->Cooking = PxCreateCooking(PX_PHYSICS_VERSION, PhysXEngine::GetFoundation(), cookingParams);
		ZE_CORE_ASSERT(s_Data->Cooking, "Failed to create PhysX Cooking!");
	}

	void PhysXCookingFactory::Shutdown()
	{
		s_Data->Cooking->release();
		s_Data->Cooking = nullptr;

		delete s_Data;
		s_Data = nullptr;
	}

}

#include "ZEpch.h"
#include "Engine/Physics/PhysXEngine.h"

#include "Engine/Physics/PhysXDebugger.h"
#include "Engine/Physics/PhysXCookingFactory.h"

namespace ZeoEngine {

	struct PhysXData
	{
		physx::PxDefaultErrorCallback DefaultErrorCallback;
		physx::PxDefaultAllocator DefaultAllocator;
		physx::PxDefaultCpuDispatcher* CPUDispatcher = nullptr;

		physx::PxFoundation* Foundation = nullptr;
		physx::PxPhysics* Physics = nullptr;

		physx::PxTolerancesScale TolerancesScale;
	};

	static PhysXData* s_Data = nullptr;

	void PhysXEngine::Init()
	{
		s_Data = new PhysXData();

		s_Data->Foundation = PxCreateFoundation(PX_PHYSICS_VERSION, s_Data->DefaultAllocator, s_Data->DefaultErrorCallback);
		ZE_CORE_ASSERT(s_Data->Foundation, "Failed to create PhysX Foundation!");

		PhysXDebugger::Init();

#ifdef ZE_DEBUG
		bool bTrackMemoryAllocations = true;
#else
		bool bTrackMemoryAllocations = false;
#endif

		s_Data->Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *s_Data->Foundation, s_Data->TolerancesScale, bTrackMemoryAllocations, PhysXDebugger::GetDebugger());
		ZE_CORE_ASSERT(s_Data->Physics, "Failed to create PhysX Physics!");

		bool bExtentionsLoaded = PxInitExtensions(*s_Data->Physics, PhysXDebugger::GetDebugger());
		ZE_CORE_ASSERT(bExtentionsLoaded, "Failed to initialize PhysX Extensions!");

		s_Data->CPUDispatcher = physx::PxDefaultCpuDispatcherCreate(1);

		PhysXCookingFactory::Init();
	}

	void PhysXEngine::Shutdown()
	{
		PhysXCookingFactory::Shutdown();

		s_Data->CPUDispatcher->release();
		s_Data->CPUDispatcher = nullptr;

		PxCloseExtensions();

		PhysXDebugger::StopDebugging();

		s_Data->Physics->release();
		s_Data->Physics = nullptr;

		PhysXDebugger::Shutdown();

		s_Data->Foundation->release();
		s_Data->Foundation = nullptr;

		delete s_Data;
		s_Data = nullptr;
	}

	physx::PxFoundation& PhysXEngine::GetFoundation()
	{
		return *s_Data->Foundation;
	}

	physx::PxTolerancesScale& PhysXEngine::GetTolerancesScale()
	{
		return s_Data->TolerancesScale;
	}

}

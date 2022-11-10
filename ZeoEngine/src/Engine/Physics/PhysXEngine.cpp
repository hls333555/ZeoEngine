#include "ZEpch.h"
#include "Engine/Physics/PhysXEngine.h"

#include "Engine/Physics/PhysXDebugger.h"
#include "Engine/Physics/PhysXCookingFactory.h"

namespace ZeoEngine {

	void PhysXErrorCallback::reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line)
	{
		const char* errorMessage = nullptr;

		switch (code)
		{
			case physx::PxErrorCode::eNO_ERROR:				errorMessage = "No Error"; break;
			case physx::PxErrorCode::eDEBUG_INFO:			errorMessage = "Info"; break;
			case physx::PxErrorCode::eDEBUG_WARNING:		errorMessage = "Warning"; break;
			case physx::PxErrorCode::eINVALID_PARAMETER:	errorMessage = "Invalid Parameter"; break;
			case physx::PxErrorCode::eINVALID_OPERATION:	errorMessage = "Invalid Operation"; break;
			case physx::PxErrorCode::eOUT_OF_MEMORY:		errorMessage = "Out Of Memory"; break;
			case physx::PxErrorCode::eINTERNAL_ERROR:		errorMessage = "Internal Error"; break;
			case physx::PxErrorCode::eABORT:				errorMessage = "Abort"; break;
			case physx::PxErrorCode::ePERF_WARNING:			errorMessage = "Performance Warning"; break;
			case physx::PxErrorCode::eMASK_ALL:				errorMessage = "Unknown Error"; break;
		}

		switch (code)
		{
			case physx::PxErrorCode::eNO_ERROR:
			case physx::PxErrorCode::eDEBUG_INFO:
				ZE_CORE_INFO("[PhysX]: {0}: {1} at {2} ({3})", errorMessage, message, file, line);
				break;
			case physx::PxErrorCode::eDEBUG_WARNING:
			case physx::PxErrorCode::ePERF_WARNING:
				ZE_CORE_WARN("[PhysX]: {0}: {1} at {2} ({3})", errorMessage, message, file, line);
				break;
			case physx::PxErrorCode::eINVALID_PARAMETER:
			case physx::PxErrorCode::eINVALID_OPERATION:
			case physx::PxErrorCode::eINTERNAL_ERROR:
				ZE_CORE_ERROR("[PhysX]: {0}: {1} at {2} ({3})", errorMessage, message, file, line);
				break;
			case physx::PxErrorCode::eOUT_OF_MEMORY:
				ZE_CORE_CRITICAL("[PhysX]: {0}: {1} at {2} ({3})", errorMessage, message, file, line);
				break;
			case physx::PxErrorCode::eABORT:
			case physx::PxErrorCode::eMASK_ALL:
				ZE_CORE_ERROR("[PhysX]: {0}: {1} at {2} ({3})", errorMessage, message, file, line);
				ZE_CORE_ASSERT(false);
				break;
		}
	}

	struct PhysXData
	{
		PhysXErrorCallback ErrorCallback;
		physx::PxDefaultAllocator DefaultAllocator;
		physx::PxDefaultCpuDispatcher* CPUDispatcher = nullptr;

		physx::PxFoundation* Foundation = nullptr;
		physx::PxPhysics* Physics = nullptr;
	};

	static PhysXData* s_Data = nullptr;

	void PhysXEngine::Init()
	{
		s_Data = new PhysXData();

		s_Data->Foundation = PxCreateFoundation(PX_PHYSICS_VERSION, s_Data->DefaultAllocator, s_Data->ErrorCallback);
		ZE_CORE_ASSERT(s_Data->Foundation, "Failed to create PhysX Foundation!");

		PhysXDebugger::Init();

#ifdef ZE_DEBUG
		bool bTrackMemoryAllocations = true;
#else
		bool bTrackMemoryAllocations = false;
#endif

		s_Data->Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *s_Data->Foundation, physx::PxTolerancesScale(), bTrackMemoryAllocations, PhysXDebugger::GetDebugger());
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

	physx::PxPhysics& PhysXEngine::GetPhysics()
	{
		return *s_Data->Physics;
	}

	physx::PxCpuDispatcher* PhysXEngine::GetCPUDispatcher()
	{
		return s_Data->CPUDispatcher;
	}

}

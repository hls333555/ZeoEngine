#include "ZEpch.h"
#include "Engine/Physics/PhysXDebugger.h"

#include <pvd/PxPvdTransport.h>

#include "Engine/Core/CommonPaths.h"
#include "Engine/Physics/PhysXEngine.h"
#include "Engine/Core/Console.h"
#include "Engine/Utils/EngineUtils.h"

namespace ZeoEngine {

	struct PhysXDebuggerData
	{
		physx::PxPvd* Debugger = nullptr;
		physx::PxPvdTransport* Transport = nullptr;
	};

	static PhysXDebuggerData* s_Data = nullptr;

	void PhysXDebugger::Init()
	{
#ifdef ZE_DEBUG
		s_Data = new PhysXDebuggerData();

		s_Data->Debugger = PxCreatePvd(PhysXEngine::GetFoundation());
		ZE_CORE_ASSERT(s_Data->Debugger, "Failed to create PhysX PVD!");

		Console::Get().RegisterCommand("physics.StartDebugger", [](const std::vector<std::string>&)
		{
			StartDebugging(PhysXDebugType::LiveDebug);
		}, "Start Nvidia physics debugger.");
		Console::Get().RegisterCommand("physics.StopDebugger", [](const std::vector<std::string>&)
		{
			StopDebugging();
		}, "Stop Nvidia physics debugger.");
#endif
	}

	void PhysXDebugger::Shutdown()
	{
#ifdef ZE_DEBUG
		s_Data->Debugger->release();
		s_Data->Debugger = nullptr;

		delete s_Data;
		s_Data = nullptr;
#endif
	}

	void PhysXDebugger::StartDebugging(PhysXDebugType debugType)
	{
#ifdef ZE_DEBUG
		StopDebugging();

		if (debugType == PhysXDebugType::LiveDebug)
		{
			s_Data->Transport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 1000);
		}
		else
		{
			
			const std::string path = fmt::format("{}/{}.pxd2", CommonPaths::GetPhysXDebuggerOutputDirectory(), EngineUtils::GetCurrentTimeAndDate());
			s_Data->Transport = physx::PxDefaultPvdFileTransportCreate(path.c_str());
		}
		s_Data->Debugger->connect(*s_Data->Transport, physx::PxPvdInstrumentationFlag::eALL);
#endif
	}

	void PhysXDebugger::StopDebugging()
	{
#ifdef ZE_DEBUG
		if (!s_Data->Debugger->isConnected()) return;

		s_Data->Debugger->disconnect();
		s_Data->Transport->release();
#endif
	}

	physx::PxPvd* PhysXDebugger::GetDebugger()
	{
#ifdef ZE_DEBUG
		return s_Data->Debugger;
#else
		return nullptr;
#endif
	}

}

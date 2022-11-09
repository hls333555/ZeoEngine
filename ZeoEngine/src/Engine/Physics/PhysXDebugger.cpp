#include "ZEpch.h"
#include "Engine/Physics/PhysXDebugger.h"

#include "Engine/Physics/PhysXEngine.h"

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

	void PhysXDebugger::StartDebugging(const std::string& filepath, bool bIsNetworking)
	{
#ifdef ZE_DEBUG
		StopDebugging();

		if (!bIsNetworking)
		{
			s_Data->Transport = physx::PxDefaultPvdFileTransportCreate((filepath + ".pxd2").c_str());
			s_Data->Debugger->connect(*s_Data->Transport, physx::PxPvdInstrumentationFlag::eALL);
		}
		else
		{
			const char* host = "127.0.0.1";
			s_Data->Transport = physx::PxDefaultPvdSocketTransportCreate(host, 5425, 1000);
			s_Data->Debugger->connect(*s_Data->Transport, physx::PxPvdInstrumentationFlag::eALL);
		}
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

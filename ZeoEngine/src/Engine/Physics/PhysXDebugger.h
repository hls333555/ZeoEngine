#pragma once

#include <PxPhysicsAPI.h>

namespace ZeoEngine {

	class PhysXDebugger
	{
		friend class PhysXEngine;

	public:
		static void Init();
		static void Shutdown();

		static void StartDebugging(const std::string& filepath, bool bIsNetworking = false);
		static void StopDebugging();

	private:
		static physx::PxPvd* GetDebugger();
	};
	
}

#pragma once

#include <pvd/PxPvd.h>

namespace ZeoEngine {

	enum class PhysXDebugType
	{
		LiveDebug,
		DebugToFile
	};

	class PhysXDebugger
	{
		friend class PhysXEngine;

	public:
		static void Init();
		static void Shutdown();

		static void StartDebugging();
		static void StartDebugging(const std::string& filepath);
		static void StopDebugging();

	private:
		static physx::PxPvd* GetDebugger();
	};
	
}

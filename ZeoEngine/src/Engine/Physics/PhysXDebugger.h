#pragma once

namespace physx
{
	class PxPvd;
}

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

		static void StartDebugging(PhysXDebugType debugType);
		static void StopDebugging();

	private:
		static physx::PxPvd* GetDebugger();
	};
	
}

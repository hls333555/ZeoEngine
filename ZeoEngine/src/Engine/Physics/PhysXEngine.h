#pragma once

#include <PxPhysicsAPI.h>

namespace ZeoEngine {

	class PhysXEngine
	{
		friend class PhysXDebugger;
		friend class PhysXCookingFactory;

	public:
		static void Init();
		static void Shutdown();

	private:
		static physx::PxFoundation& GetFoundation();
		static physx::PxTolerancesScale& GetTolerancesScale();
	};

}

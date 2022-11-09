#pragma once

#include <PxPhysicsAPI.h>

namespace ZeoEngine {

	class PhysXErrorCallback : public physx::PxErrorCallback
	{
	public:
		virtual void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line) override;
	};

	class PhysXEngine
	{
		friend class PhysXDebugger;
		friend class PhysXCookingFactory;
		friend class PhysXScene;

	public:
		static void Init();
		static void Shutdown();

	private:
		static physx::PxFoundation& GetFoundation();
		static physx::PxPhysics& GetPhysics();
		static physx::PxCpuDispatcher* GetCPUDispatcher();
	};

}

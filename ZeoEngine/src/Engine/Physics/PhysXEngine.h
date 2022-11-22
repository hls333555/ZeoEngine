#pragma once

#include <foundation/PxErrorCallback.h>

namespace physx
{
	class PxCpuDispatcher;
	class PxPhysics;
	class PxFoundation;
}

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
		friend class PhysXActor;
		friend class PhysicsMaterial;
		friend class PhysXUtils;

	public:
		static void Init();
		static void Shutdown();

	private:
		static physx::PxFoundation& GetFoundation();
		static physx::PxPhysics& GetPhysics();
		static physx::PxCpuDispatcher* GetCPUDispatcher();
		static Ref<PhysicsMaterial> GetDefaultPhysicsMaterial();
	};

}

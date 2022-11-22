#pragma once

#include "Engine/Asset/Asset.h"

namespace physx
{
	class PxMaterial;
}

namespace ZeoEngine {

	class PhysicsMaterial : public AssetBase<PhysicsMaterial>
	{
		friend struct PhysicsMaterialDetailComponent;
		friend class PhysXUtils;

	public:
		PhysicsMaterial();
		~PhysicsMaterial();

		static Ref<PhysicsMaterial> Create();

	private:
		physx::PxMaterial* m_PhysicsMaterial = nullptr;

		float m_StaticFriction = 0.6f;
		float m_DynamicFriction = 0.6f;
		float m_Bounciness = 0.0f;
	};
	
}

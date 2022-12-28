#include "ZEpch.h"
#include "Engine/Physics/PhysicsMaterial.h"

#include <PxMaterial.h>
#include <PxPhysics.h>

#include "Engine/Physics/PhysXEngine.h"

namespace ZeoEngine {

	PhysicsMaterial::PhysicsMaterial()
	{
		m_PhysicsMaterial = PhysXEngine::GetPhysics().createMaterial(m_StaticFriction, m_DynamicFriction, m_Bounciness);
		m_PhysicsMaterial->userData = this;
	}

	PhysicsMaterial::~PhysicsMaterial()
	{
		m_PhysicsMaterial->release();
	}

	Ref<PhysicsMaterial> PhysicsMaterial::Create()
	{
		return CreateRef<PhysicsMaterial>();
	}

}

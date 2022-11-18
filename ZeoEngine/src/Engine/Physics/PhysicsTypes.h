#pragma once

#include "Engine/Asset/Asset.h"

#define PHYSICS_MAX_CONTACT_POINTS 16

namespace ZeoEngine {

	class PhysXColliderShapeBase;

	enum class BroadphaseType
	{
		SweepAndPrune,
		MultiBoxPrune,
		AutomaticBoxPrune
	};

	enum class ForceMode
	{
		Force = 0,
		Impulse,
		VelocityChange,
		Acceleration
	};

	enum class ColliderType
	{
		Box, Sphere, Capsule, ConvexMesh, TriangleMesh
	};

	struct ContactInfo
	{
		Vec3 ContactPosition;
		Vec3 ContactNormal;
		float Separation;
		AssetHandle PhysicsMaterialAssets[2];
	};

	struct CollisionInfo
	{
		PhysXColliderShapeBase* OtherCollider = nullptr;
		U32 NumContacts = 0;
		ContactInfo Contacts[PHYSICS_MAX_CONTACT_POINTS];
	};

}

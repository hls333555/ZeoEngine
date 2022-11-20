#pragma once

#include "Engine/Asset/Asset.h"

#define PHYSICS_MAX_CONTACT_POINTS 16

namespace ZeoEngine {

	class PhysXColliderShapeBase;

	enum class BroadphaseType
	{
		SweepAndPrune = 0,
		MultiBoxPrune,
		AutomaticBoxPrune
	};

	enum class FrictionType
	{
		Patch = 0,
		OneDirectional,
		TwoDirectional
	};

	enum class SolverType
	{
		ProjectedGaussSeidel,
		TemporalGaussSeidel
	};

	enum class ActorLockFlag
	{
		TranslationX = ZE_BIT(0), TranslationY = ZE_BIT(1), TranslationZ = ZE_BIT(2), Translation = TranslationX | TranslationY | TranslationZ,
		RotationX = ZE_BIT(3), RotationY = ZE_BIT(4), RotationZ = ZE_BIT(5), Rotation = RotationX | RotationY | RotationZ
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

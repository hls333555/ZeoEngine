#pragma once

namespace ZeoEngine {

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

}

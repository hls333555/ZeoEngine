#include "ZEpch.h"
#include "Engine/Math/Math.h"

#include <glm/gtx/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

namespace ZeoEngine::Math {

	Vec3 GetForwardVector(const Vec3& rotation)
	{
		return glm::rotate(glm::quat(rotation), { 0.0f, 0.0f, -1.0f });
	}

	Vec3 GetRightVector(const Vec3& rotation)
	{
		return glm::rotate(glm::quat(rotation), { 1.0f, 0.0f, 0.0f });
	}

	Vec3 GetUpVector(const Vec3& rotation)
	{
		return glm::rotate(glm::quat(rotation), { 0.0f, 1.0f, 0.0f });
	}

	bool DecomposeTransform(const Mat4& transform, Vec3& outTranslation, Vec3& outRotation, Vec3& outScale)
	{
		// From glm::decompose in matrix_decompose.inl

		using namespace glm;
		using T = float;

		mat4 LocalMatrix(transform);

		// Normalize the matrix.
		if (epsilonEqual(LocalMatrix[3][3], static_cast<float>(0), epsilon<T>())) return false;

		// First, isolate perspective.  This is the messiest.
		if (
			epsilonNotEqual(LocalMatrix[0][3], static_cast<T>(0), epsilon<T>()) ||
			epsilonNotEqual(LocalMatrix[1][3], static_cast<T>(0), epsilon<T>()) ||
			epsilonNotEqual(LocalMatrix[2][3], static_cast<T>(0), epsilon<T>()))
		{
			// Clear the perspective partition
			LocalMatrix[0][3] = LocalMatrix[1][3] = LocalMatrix[2][3] = static_cast<T>(0);
			LocalMatrix[3][3] = static_cast<T>(1);
		}

		// Next take care of translation (easy).
		outTranslation = vec3(LocalMatrix[3]);
		LocalMatrix[3] = vec4(0, 0, 0, LocalMatrix[3].w);

		vec3 Row[3];

		// Now get scale and shear.
		for (length_t i = 0; i < 3; ++i)
		{
			for (length_t j = 0; j < 3; ++j)
			{
				Row[i][j] = LocalMatrix[i][j];
			}
		}

		// Compute X scale factor and normalize first row.
		outScale.x = length(Row[0]);
		Row[0] = detail::scale(Row[0], static_cast<T>(1));
		outScale.y = length(Row[1]);
		Row[1] = detail::scale(Row[1], static_cast<T>(1));
		outScale.z = length(Row[2]);
		Row[2] = detail::scale(Row[2], static_cast<T>(1));

		// At this point, the matrix (in rows[]) is orthonormal.
		// Check for a coordinate system flip.  If the determinant
		// is -1, then negate the matrix and the scaling factors.
#if 0
		Pdum3 = cross(Row[1], Row[2]); // v3Cross(row[1], row[2], Pdum3);
		if (dot(Row[0], Pdum3) < 0)
		{
			for (length_t i = 0; i < 3; i++)
			{
				scale[i] *= static_cast<T>(-1);
				Row[i] *= static_cast<T>(-1);
			}
		}
#endif

		outRotation.y = asin(-Row[0][2]);
		if (cos(outRotation.y) != 0)
		{
			outRotation.x = atan2(Row[1][2], Row[2][2]);
			outRotation.z = atan2(Row[0][1], Row[0][0]);
		}
		else
		{
			outRotation.x = atan2(-Row[2][0], Row[1][1]);
			outRotation.z = 0;
		}

		return true;
	}

	Vec3 GetTranslationFromTransform(const Mat4& transform)
	{
		return Vec3(transform[3]);
	}

	float FInterpTo(float current, float target, DeltaTime dt, float interpSpeed)
	{
		// If no interp speed, jump to target value
		if (interpSpeed <= 0.f)
		{
			return target;
		}

		// Distance to reach
		const float dist = target - current;

		// If distance is too small, just set the desired location
		if (dist * dist < SMALL_NUMBER)
		{
			return target;
		}

		// Delta Move, Clamp so we do not over shoot.
		const float deltaMove = dist * glm::clamp<float>(dt * interpSpeed, 0.0f, 1.0f);

		return current + deltaMove;
	}

	Vec3 VInterpTo(const Vec3& current, const Vec3& target, DeltaTime dt, float interpSpeed)
	{
		// If no interp speed, jump to target value
		if (interpSpeed <= 0.0f)
		{
			return target;
		}

		// Distance to reach
		const Vec3 dist = target - current;

		// If distance is too small, just set the desired location
		if (glm::length(dist) < KINDA_SMALL_NUMBER)
		{
			return target;
		}

		// Delta Move, clamp so we do not over shoot
		const Vec3 deltaMove = dist * glm::clamp<float>(dt * interpSpeed, 0.0f, 1.0f);

		return current + deltaMove;
	}

	Vec3 InterpConstantTo(const Vec3& current, const Vec3& target, DeltaTime dt, float interpSpeed)
	{
		const Vec3 delta = target - current;
		const float deltaM = glm::length(delta);
		const float maxStep = interpSpeed * dt;

		if (deltaM > maxStep)
		{
			if (maxStep > 0.f)
			{
				const Vec3 deltaN = delta / deltaM;
				return current + deltaN * maxStep;
			}
			else
			{
				return current;
			}
		}

		return target;
	}

	bool IsNearlyEqual(float x, float y, float errorTolerance)
	{
		return glm::abs(x - y) <= errorTolerance;
	}

	bool IsNearlyEqual(double x, double y, double errorTolerance)
	{
		return  glm::abs(x - y) <= errorTolerance;
	}

}

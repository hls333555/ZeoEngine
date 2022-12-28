#pragma once

#include "Engine/Core/DeltaTime.h"

namespace ZeoEngine::Math {

#define SMALL_NUMBER				(1.e-8f)
#define KINDA_SMALL_NUMBER			(1.e-4f)
#define DOUBLE_SMALL_NUMBER			(1.e-8)
#define DOUBLE_KINDA_SMALL_NUMBER	(1.e-4)

	Vec3 GetForwardVector(const Vec3& rotation);
	Vec3 GetRightVector(const Vec3& rotation);
	Vec3 GetUpVector(const Vec3& rotation);

	Mat4 ComposeTransform(const Vec3& translation, const Vec3& rotation, const Vec3& scale);
	bool DecomposeTransform(const Mat4& transform, Vec3& outTranslation, Vec3& outRotation, Vec3& outScale);
	Vec3 GetTranslationFromTransform(const Mat4& transform);

	/** Interpolate from current to target. Scaled by distance to Target, so it has a strong start speed and ease out. */
	[[nodiscard]] float InterpTo(float current, float target, DeltaTime dt, float interpSpeed);
	template<typename T>
	[[nodiscard]] T InterpTo(const T& current, const T& target, DeltaTime dt, float interpSpeed)
	{
		static_assert(std::is_same_v<T, Vec2> || std::is_same_v<T, Vec3>);

		// If no interp speed, jump to target value
		if (interpSpeed <= 0.0f)
		{
			return target;
		}

		// Distance to reach
		const T dist = target - current;

		// If distance is too small, just set the desired location
		if (glm::length2(dist) < KINDA_SMALL_NUMBER)
		{
			return target;
		}

		// Delta Move, clamp so we do not over shoot
		const T deltaMove = dist * glm::clamp<float>(dt * interpSpeed, 0.0f, 1.0f);

		return current + deltaMove;
	}
	/** Interpolate from current to target with constant step. */
	[[nodiscard]] float InterpConstantTo(float current, float target, DeltaTime dt, float interpSpeed);
	template<typename T>
	[[nodiscard]] T InterpConstantTo(const T& current, const T& target, DeltaTime dt, float interpSpeed)
	{
		static_assert(std::is_same_v<T, Vec2> || std::is_same_v<T, Vec3>);

		const T delta = target - current;
		const float deltaM = glm::length(delta);
		const float maxStep = interpSpeed * dt;

		if (deltaM > maxStep)
		{
			if (maxStep > 0.0f)
			{
				const T deltaN = delta / deltaM;
				return current + deltaN * maxStep;
			}
			return current;
		}

		return target;
	}

	bool IsNearlyEqual(float x, float y, float errorTolerance = SMALL_NUMBER);
	bool IsNearlyEqual(double x, double y, double errorTolerance = DOUBLE_SMALL_NUMBER);
	bool IsNearlyEqual(const Vec2& x, const Vec2& y, float errorTolerance = KINDA_SMALL_NUMBER);
	bool IsNearlyEqual(const Vec3& x, const Vec3& y, float errorTolerance = KINDA_SMALL_NUMBER);

	/** Calculate a rotation (in radians) which makes forward (local -Z) at "from" position to point at "to" position. */
	Vec3 FindLookAtRotation(const Vec3& from, const Vec3& to, const Vec3& up);

}

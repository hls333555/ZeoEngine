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

	/** Interpolate float from current to target. Scaled by distance to Target, so it has a strong start speed and ease out. */
	float FInterpTo(float current, float target, DeltaTime dt, float interpSpeed);
	/** Interpolate vector from current to target. Scaled by distance to Target, so it has a strong start speed and ease out. */
	Vec3 VInterpTo(const Vec3& current, const Vec3& target, DeltaTime dt, float interpSpeed);
	Vec3 VInterpConstantTo(const Vec3& current, const Vec3& target, DeltaTime dt, float interpSpeed);

	bool IsNearlyEqual(float x, float y, float errorTolerance = SMALL_NUMBER);
	bool IsNearlyEqual(double x, double y, double errorTolerance = DOUBLE_SMALL_NUMBER);
	bool IsNearlyEqual(const Vec3& x, const Vec3& y, float errorTolerance = KINDA_SMALL_NUMBER);

	Vec3 FindLookAtRotation(const Vec3& from, const Vec3& to, const Vec3& up);

}

#include "ZEpch.h"

#define DEBUG_DRAW_EXPLICIT_CONTEXT
#define DEBUG_DRAW_IMPLEMENTATION
#include <debug_draw.hpp>

namespace dd {

	static inline void vecCross(ddVec3_Out v, ddVec3_In v1, ddVec3_In v2)
	{
		v[X] = v2[Y] * v1[Z] - v1[Y] * v2[Z];
		v[Y] = v2[Z] * v1[X] - v1[Z] * v2[X];
		v[Z] = v2[X] * v1[Y] - v1[X] * v2[Y];
		vecNormalize(v, v);
	}

	void capsule(DD_EXPLICIT_CONTEXT_ONLY(ContextHandle ctx, ) ddVec3_In center, ddVec3_In planeNormal, ddVec3_In planeDirection, ddVec3_In color, const float radius, const float height, const float numSteps, const int durationMillis, const bool depthEnabled)
	{
		if (!isInitialized(DD_EXPLICIT_CONTEXT_ONLY(ctx)))
		{
			return;
		}

		ddVec3 left, upRadius, upHalfHeight;
		ddVec3 point, lastPoint, upCenter, downCenter;

		vecCross(left, planeDirection, planeNormal);

		float halfHeight = height * 0.5f;
		vecScale(upHalfHeight, planeDirection, halfHeight);
		vecScale(upRadius, planeDirection, radius);
		vecScale(left, left, radius);
		vecAdd(upCenter, center, upHalfHeight);
		vecSub(downCenter, center, upHalfHeight);
		vecAdd(lastPoint, upCenter, upRadius);
		
		// Draw upper 1/4 circle
		for (int i = 1; i <= numSteps / 4; ++i)
		{
			const float radians = TAU * i / numSteps;

			ddVec3 vs, vc;
			vecScale(vs, left, floatSin(radians));
			vecScale(vc, upRadius, floatCos(radians));

			vecAdd(point, upCenter, vs);
			vecAdd(point, point, vc);

			line(DD_EXPLICIT_CONTEXT_ONLY(ctx, ) lastPoint, point, color, durationMillis, depthEnabled);
			vecCopy(lastPoint, point);
		}

		// Draw vertical line from up to bottom
		vecSub(point, point, upHalfHeight);
		line(DD_EXPLICIT_CONTEXT_ONLY(ctx, ) lastPoint, point, color, durationMillis, depthEnabled);
		vecCopy(lastPoint, point);

		// Draw bottom 1/2 circle
		for (int i = static_cast<int>(numSteps) / 4 + 1; i <= numSteps / 4 * 3; ++i)
		{
			const float radians = TAU * i / numSteps;

			ddVec3 vs, vc;
			vecScale(vs, left, floatSin(radians));
			vecScale(vc, upRadius, floatCos(radians));

			vecAdd(point, downCenter, vs);
			vecAdd(point, point, vc);

			line(DD_EXPLICIT_CONTEXT_ONLY(ctx, ) lastPoint, point, color, durationMillis, depthEnabled);
			vecCopy(lastPoint, point);
		}

		// Draw vertical line from bottom to up
		vecAdd(point, point, upHalfHeight);
		line(DD_EXPLICIT_CONTEXT_ONLY(ctx, ) lastPoint, point, color, durationMillis, depthEnabled);
		vecCopy(lastPoint, point);

		// Draw upper 1/4 circle
		for (int i = static_cast<int>(numSteps) / 4 * 3 + 1; i <= numSteps; ++i)
		{
			const float radians = TAU * i / numSteps;

			ddVec3 vs, vc;
			vecScale(vs, left, floatSin(radians));
			vecScale(vc, upRadius, floatCos(radians));

			vecAdd(point, upCenter, vs);
			vecAdd(point, point, vc);

			line(DD_EXPLICIT_CONTEXT_ONLY(ctx, ) lastPoint, point, color, durationMillis, depthEnabled);
			vecCopy(lastPoint, point);
		}
	}

}

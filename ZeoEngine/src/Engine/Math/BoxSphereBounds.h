#pragma once

#include <vector>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "Engine/Math/Math.h"

namespace ZeoEngine {

	/** An axis-aligned box */
	struct Box
	{
		Vec3 Min{ 0.0f };
		Vec3 Max{ 0.0f };
		bool bIsValid = false;
		
		Box() = default;
		Box(const Vec3& min, const Vec3& max)
			: Min(min), Max(max), bIsValid(true) {}

		Vec3 GetExtent() const
		{
			return 0.5f * (Max - Min);
		}

		void GetCenterAndExtents(Vec3& center, Vec3& extents) const
		{
			extents = GetExtent();
			center = Min + extents;
		}

		Box& operator+=(const Vec3& Other)
		{
			if (bIsValid)
			{
				Min.x = glm::min(Min.x, Other.x);
				Min.y = glm::min(Min.y, Other.y);
				Min.z = glm::min(Min.z, Other.z);

				Max.x = glm::max(Max.x, Other.x);
				Max.y = glm::max(Max.y, Other.y);
				Max.z = glm::max(Max.z, Other.z);
			}
			else
			{
				Min = Max = Other;
				bIsValid = true;
			}

			return *this;
		}

		Box operator+(const Vec3& Other) const
		{
			return Box(*this) += Other;
		}

		Box& Box::operator+=(const Box& Other)
		{
			if (bIsValid && Other.bIsValid)
			{
				Min.x = glm::min(Min.x, Other.Min.x);
				Min.y = glm::min(Min.y, Other.Min.y);
				Min.z = glm::min(Min.z, Other.Min.z);

				Max.x = glm::max(Max.x, Other.Max.x);
				Max.y = glm::max(Max.y, Other.Max.y);
				Max.z = glm::max(Max.z, Other.Max.z);
			}
			else if (Other.bIsValid)
			{
				*this = Other;
			}

			return *this;
		}

		Box operator+(const Box& Other) const
		{
			return Box(*this) += Other;
		}

	};

	struct Sphere
	{
		Vec3 Center;
		float Radius;
	};

	/** A combined axis aligned bounding box and bounding sphere with the same origin */
	struct BoxSphereBounds
	{
		Vec3 Origin{ 0.0f };
		Vec3 BoxExtent{ 0.0f };
		float SphereRadius = 0.0f;

		BoxSphereBounds() = default;

		BoxSphereBounds(const Vec3& origin, const Vec3& boxExtent, float sphereRadius)
			: Origin(origin), BoxExtent(boxExtent), SphereRadius(sphereRadius) {}

		BoxSphereBounds(const Box& box)
		{
			box.GetCenterAndExtents(Origin, BoxExtent);
			SphereRadius = (BoxExtent.x == BoxExtent.y && BoxExtent.x == BoxExtent.z) ? BoxExtent.x : glm::length(BoxExtent);
		}

		BoxSphereBounds(const Sphere& Sphere)
		{
			Origin = Sphere.Center;
			BoxExtent = Vec3(Sphere.Radius);
			SphereRadius = Sphere.Radius;
		}

		BoxSphereBounds(const std::vector<Vec3>& points)
		{
			Box boundingBox;
			for (const auto& point : points)
			{
				boundingBox += point;
			}
			boundingBox.GetCenterAndExtents(Origin, BoxExtent);
			float sphereRadius = 0.0f;
			for (const auto& point : points)
			{
				sphereRadius = glm::max(sphereRadius, glm::length(point - Origin));
			}
		}

		BoxSphereBounds operator+(const BoxSphereBounds& Other) const
		{
			Box boundingBox;

			boundingBox += (Origin - BoxExtent);
			boundingBox += (Origin + BoxExtent);
			boundingBox += (Other.Origin - Other.BoxExtent);
			boundingBox += (Other.Origin + Other.BoxExtent);

			BoxSphereBounds result(boundingBox);
			result.SphereRadius = glm::min(result.SphereRadius, glm::max(glm::length(Origin - result.Origin) + SphereRadius, glm::length(Other.Origin - result.Origin) + Other.SphereRadius));

			return result;
		}

		[[nodiscard]]BoxSphereBounds TransformBy(const Mat4& transform) const
		{
			BoxSphereBounds result;

			Vec3 translation, rotation, scale;
			Math::DecomposeTransform(transform, translation, rotation, scale);
			Mat4 rotationMatrix = glm::toMat4(glm::quat(rotation));
			Mat4 scaleMatrix = glm::scale(Mat4(1.0f), scale);

			result.Origin = transform * Vec4(Origin, 1.0f);
			result.BoxExtent = rotationMatrix * scaleMatrix * Vec4(BoxExtent, 1.0f);
			result.SphereRadius = glm::length(result.BoxExtent);

			return result;
		}

		bool IsValid() const
		{
			return BoxExtent != Vec3(0.0f) && SphereRadius != 0.0f;
		}

		Box GetBox() const
		{
			return IsValid() ? Box(Origin - BoxExtent, Origin + BoxExtent) : Box();
		}
	};

}

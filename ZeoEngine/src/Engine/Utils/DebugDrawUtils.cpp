#include "ZEpch.h"
#include "Engine/Utils/DebugDrawUtils.h"

#include <debug_draw.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace ZeoEngine {

	void DebugDrawUtils::DrawPoint(const glm::vec3& position, const glm::vec3& color, float size, float duration)
	{
		dd::point(glm::value_ptr(position), glm::value_ptr(color), size, static_cast<int32_t>(duration * 1000.0f));
	}

	void DebugDrawUtils::DrawLine(const glm::vec3& from, const glm::vec3& to, const glm::vec3& color, float duration)
	{
		dd::line(glm::value_ptr(from), glm::value_ptr(to), glm::value_ptr(color), static_cast<int32_t>(duration * 1000.0f));
	}

	void DebugDrawUtils::DrawBox(const glm::vec3& center, const glm::vec3& extent, const glm::vec3& color, const glm::vec3& rotation, float duration)
	{
		if (rotation == glm::vec3(0.0f))
		{
			dd::box(glm::value_ptr(center), glm::value_ptr(color), extent.x, extent.y, extent.z, static_cast<int32_t>(duration * 1000.0f));
		}
		else
		{
			ddVec3 points[8];
			auto point = center + glm::vec3(glm::toMat4(glm::quat(rotation)) * glm::vec4(-extent.x, extent.y, extent.z, 1.0f));
			points[0][0] = point.x; points[0][1] = point.y; points[0][2] = point.z;
			point = center + glm::vec3(glm::toMat4(glm::quat(rotation)) * glm::vec4(-extent.x, extent.y, -extent.z, 1.0f));
			points[1][0] = point.x; points[1][1] = point.y; points[1][2] = point.z;
			point = center + glm::vec3(glm::toMat4(glm::quat(rotation)) * glm::vec4(extent.x, extent.y, -extent.z, 1.0f));
			points[2][0] = point.x; points[2][1] = point.y; points[2][2] = point.z;
			point = center + glm::vec3(glm::toMat4(glm::quat(rotation)) * glm::vec4(extent.x, extent.y, extent.z, 1.0f));
			points[3][0] = point.x; points[3][1] = point.y; points[3][2] = point.z;
			point = center + glm::vec3(glm::toMat4(glm::quat(rotation)) * glm::vec4(-extent.x, -extent.y, extent.z, 1.0f));
			points[4][0] = point.x; points[4][1] = point.y; points[4][2] = point.z;
			point = center + glm::vec3(glm::toMat4(glm::quat(rotation)) * glm::vec4(-extent.x, -extent.y, -extent.z, 1.0f));
			points[5][0] = point.x; points[5][1] = point.y; points[5][2] = point.z;
			point = center + glm::vec3(glm::toMat4(glm::quat(rotation)) * glm::vec4(extent.x, -extent.y, -extent.z, 1.0f));
			points[6][0] = point.x; points[6][1] = point.y; points[6][2] = point.z;
			point = center + glm::vec3(glm::toMat4(glm::quat(rotation)) * glm::vec4(extent.x, -extent.y, extent.z, 1.0f));
			points[7][0] = point.x; points[7][1] = point.y; points[7][2] = point.z;
			dd::box(points, glm::value_ptr(color), static_cast<int32_t>(duration * 1000.0f));
		}
	}

	void DebugDrawUtils::DrawCircle(const glm::vec3& center, const glm::vec3& planeNormal, const glm::vec3& color, float radius, float segaments, float duration)
	{
		dd::circle(glm::value_ptr(center), glm::value_ptr(planeNormal), glm::value_ptr(color), radius, segaments, static_cast<int32_t>(duration * 1000.0f));
	}

	void DebugDrawUtils::DrawSphereBounds(const glm::vec3& center, const glm::vec3& color, float radius, float segaments, float duration)
	{
		dd::circle(glm::value_ptr(center), glm::value_ptr(glm::vec3{ 1.0f, 0.0f, 0.0f }), glm::value_ptr(color), radius, segaments, static_cast<int32_t>(duration * 1000.0f));
		dd::circle(glm::value_ptr(center), glm::value_ptr(glm::vec3{ 0.0f, 1.0f, 0.0f }), glm::value_ptr(color), radius, segaments, static_cast<int32_t>(duration * 1000.0f));
		dd::circle(glm::value_ptr(center), glm::value_ptr(glm::vec3{ 0.0f, 0.0f, 1.0f }), glm::value_ptr(color), radius, segaments, static_cast<int32_t>(duration * 1000.0f));
	}

	void DebugDrawUtils::DrawSphere(const glm::vec3& center, const glm::vec3& color, float radius, float duration)
	{
		dd::sphere(glm::value_ptr(center), glm::value_ptr(color), radius, static_cast<int32_t>(duration * 1000.0f));
	}

	void DebugDrawUtils::DrawArrow(const glm::vec3& from, const glm::vec3& to, const glm::vec3& color, float size, float duration)
	{
		dd::arrow(glm::value_ptr(from), glm::value_ptr(to), glm::value_ptr(color), size, static_cast<int32_t>(duration * 1000.0f));
	}

	void DebugDrawUtils::DrawCone(const glm::vec3& apex, const glm::vec3& direction, const glm::vec3& color, float baseRadius, float apexRadius, float duration)
	{
		dd::cone(glm::value_ptr(apex), glm::value_ptr(direction), glm::value_ptr(color), baseRadius, apexRadius, static_cast<int32_t>(duration * 1000.0f));
	}

	void DebugDrawUtils::DrawPlane(const glm::vec3& center, const glm::vec3& planeNormal, const glm::vec3& planeColor, const glm::vec3& normalVecColor, float planeScale, float normalVecScale, float duration)
	{
		dd::plane(glm::value_ptr(center), glm::value_ptr(planeNormal), glm::value_ptr(planeColor), glm::value_ptr(normalVecColor), planeScale, normalVecScale, static_cast<int32_t>(duration * 1000.0f));
	}

	void DebugDrawUtils::DrawFrustum(const glm::mat4& invClipMatrix, const glm::vec3& color, float duration)
	{
		dd::frustum(glm::value_ptr(invClipMatrix), glm::value_ptr(color), static_cast<int32_t>(duration * 1000.0f));
	}
}

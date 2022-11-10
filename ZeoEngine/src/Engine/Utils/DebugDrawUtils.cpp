#include "ZEpch.h"
#include "Engine/Utils/DebugDrawUtils.h"

#define DEBUG_DRAW_EXPLICIT_CONTEXT
#include <debug_draw.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Engine/GameFramework/Scene.h"

namespace ZeoEngine {

	void DebugDrawUtils::DrawPoint(const Scene& scene, const Vec3& position, const Vec3& color, float size, float duration)
	{
		dd::point(scene.GetContextShared()->DebugDrawContext, glm::value_ptr(position), glm::value_ptr(color), size, static_cast<I32>(duration * 1000.0f));
	}

	void DebugDrawUtils::DrawLine(const Scene& scene, const Vec3& from, const Vec3& to, const Vec3& color, float duration)
	{
		dd::line(scene.GetContextShared()->DebugDrawContext, glm::value_ptr(from), glm::value_ptr(to), glm::value_ptr(color), static_cast<I32>(duration * 1000.0f));
	}

	void DebugDrawUtils::DrawBox(const Scene& scene, const Vec3& center, const Vec3& extent, const Vec3& color, const Vec3& rotation, float duration)
	{
		ddVec3 points[8];
		Mat4 transform = glm::translate(Mat4(1.0f), center) * glm::toMat4(Quat(rotation)) * glm::scale(Mat4(1.0f), Vec3(extent.x, extent.y, extent.z));
		auto point = transform * Vec4(-0.5f, 0.5f, 0.5f, 1);
		points[0][0] = point.x; points[0][1] = point.y; points[0][2] = point.z;
		point = transform * Vec4(-0.5f, 0.5f, -0.5f, 1);
		points[1][0] = point.x; points[1][1] = point.y; points[1][2] = point.z;
		point = transform * Vec4(0.5f, 0.5f, -0.5f, 1);
		points[2][0] = point.x; points[2][1] = point.y; points[2][2] = point.z;
		point = transform * Vec4(0.5f, 0.5f, 0.5f, 1);
		points[3][0] = point.x; points[3][1] = point.y; points[3][2] = point.z;
		point = transform * Vec4(-0.5f, -0.5f, 0.5f, 1);
		points[4][0] = point.x; points[4][1] = point.y; points[4][2] = point.z;
		point = transform * Vec4(-0.5f, -0.5f, -0.5f, 1);
		points[5][0] = point.x; points[5][1] = point.y; points[5][2] = point.z;
		point = transform * Vec4(0.5f, -0.5f, -0.5f, 1);
		points[6][0] = point.x; points[6][1] = point.y; points[6][2] = point.z;
		point = transform * Vec4(0.5f, -0.5f, 0.5f, 1);
		points[7][0] = point.x; points[7][1] = point.y; points[7][2] = point.z;
		dd::box(scene.GetContextShared()->DebugDrawContext, points, glm::value_ptr(color), static_cast<I32>(duration * 1000.0f));
	}

	void DebugDrawUtils::DrawCircle(const Scene& scene, const Vec3& center, const Vec3& planeNormal, const Vec3& color, float radius, float segaments, float duration)
	{
		dd::circle(scene.GetContextShared()->DebugDrawContext, glm::value_ptr(center), glm::value_ptr(planeNormal), glm::value_ptr(color), radius, segaments, static_cast<I32>(duration * 1000.0f));
	}

	void DebugDrawUtils::DrawSphereBounds(const Scene& scene, const Vec3& center, const Vec3& color, float radius, float segaments, float duration)
	{
		dd::circle(scene.GetContextShared()->DebugDrawContext, glm::value_ptr(center), glm::value_ptr(Vec3{ 1.0f, 0.0f, 0.0f }), glm::value_ptr(color), radius, segaments, static_cast<I32>(duration * 1000.0f));
		dd::circle(scene.GetContextShared()->DebugDrawContext, glm::value_ptr(center), glm::value_ptr(Vec3{ 0.0f, 1.0f, 0.0f }), glm::value_ptr(color), radius, segaments, static_cast<I32>(duration * 1000.0f));
		dd::circle(scene.GetContextShared()->DebugDrawContext, glm::value_ptr(center), glm::value_ptr(Vec3{ 0.0f, 0.0f, 1.0f }), glm::value_ptr(color), radius, segaments, static_cast<I32>(duration * 1000.0f));
	}

	void DebugDrawUtils::DrawSphere(const Scene& scene, const Vec3& center, const Vec3& color, float radius, float duration)
	{
		dd::sphere(scene.GetContextShared()->DebugDrawContext, glm::value_ptr(center), glm::value_ptr(color), radius, static_cast<I32>(duration * 1000.0f));
	}

	void DebugDrawUtils::DrawArrow(const Scene& scene, const Vec3& from, const Vec3& to, const Vec3& color, float size, float duration)
	{
		dd::arrow(scene.GetContextShared()->DebugDrawContext, glm::value_ptr(from), glm::value_ptr(to), glm::value_ptr(color), size, static_cast<I32>(duration * 1000.0f));
	}

	void DebugDrawUtils::DrawCone(const Scene& scene, const Vec3& apex, const Vec3& direction, const Vec3& color, float baseRadius, float apexRadius, float duration)
	{
		dd::cone(scene.GetContextShared()->DebugDrawContext, glm::value_ptr(apex), glm::value_ptr(direction), glm::value_ptr(color), baseRadius, apexRadius, static_cast<I32>(duration * 1000.0f));
	}

	void DebugDrawUtils::DrawPlane(const Scene& scene, const Vec3& center, const Vec3& planeNormal, const Vec3& planeColor, const Vec3& normalVecColor, float planeScale, float normalVecScale, float duration)
	{
		dd::plane(scene.GetContextShared()->DebugDrawContext, glm::value_ptr(center), glm::value_ptr(planeNormal), glm::value_ptr(planeColor), glm::value_ptr(normalVecColor), planeScale, normalVecScale, static_cast<I32>(duration * 1000.0f));
	}

	void DebugDrawUtils::DrawFrustum(const Scene& scene, const Mat4& invClipMatrix, const Vec3& color, float duration)
	{
		dd::frustum(scene.GetContextShared()->DebugDrawContext, glm::value_ptr(invClipMatrix), glm::value_ptr(color), static_cast<I32>(duration * 1000.0f));
	}
}

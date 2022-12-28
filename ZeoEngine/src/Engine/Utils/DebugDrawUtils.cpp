#include "ZEpch.h"
#include "Engine/Utils/DebugDrawUtils.h"

#define DEBUG_DRAW_EXPLICIT_CONTEXT
#include <debug_draw.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Engine/GameFramework/Scene.h"

namespace dd {

	void capsule(DD_EXPLICIT_CONTEXT_ONLY(ContextHandle ctx, ) ddVec3_In center, ddVec3_In planeNormal, ddVec3_In planeDirection, ddVec3_In color, const float radius, const float height, const float numSteps, const int durationMillis, const bool depthEnabled = true);

}

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
		const Mat4 transform = glm::translate(Mat4(1.0f), center) * glm::toMat4(Quat(rotation)) * glm::scale(Mat4(1.0f), Vec3(extent.x, extent.y, extent.z));
		DrawBox(scene, transform, color, duration);
	}

	void DebugDrawUtils::DrawBox(const Scene& scene, const Mat4& transform, const Vec3& color, float duration)
	{
		ddVec3 points[8];
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

	void DebugDrawUtils::DrawSphereBounds(const Scene& scene, const Vec3& center, const Vec3& color, float radius, const Vec3& rotation, float segaments, float duration)
	{
		const Mat4 rotationMatrix = glm::toMat4(Quat(rotation));
		const Vec4 up = rotationMatrix * Vec4{ 0.0f, 1.0f, 0.0f, 0.0f };
		const Vec4 right = rotationMatrix * Vec4{ 1.0f, 0.0f, 0.0f, 0.0f };
		const Vec4 front = rotationMatrix * Vec4{ 0.0f, 0.0f, -1.0f, 0.0f };
		dd::circle(scene.GetContextShared()->DebugDrawContext, glm::value_ptr(center), glm::value_ptr(right), glm::value_ptr(color), radius, segaments, static_cast<I32>(duration * 1000.0f));
		dd::circle(scene.GetContextShared()->DebugDrawContext, glm::value_ptr(center), glm::value_ptr(up), glm::value_ptr(color), radius, segaments, static_cast<I32>(duration * 1000.0f));
		dd::circle(scene.GetContextShared()->DebugDrawContext, glm::value_ptr(center), glm::value_ptr(front), glm::value_ptr(color), radius, segaments, static_cast<I32>(duration * 1000.0f));
	}

	void DebugDrawUtils::DrawSphere(const Scene& scene, const Vec3& center, const Vec3& color, float radius, float duration)
	{
		dd::sphere(scene.GetContextShared()->DebugDrawContext, glm::value_ptr(center), glm::value_ptr(color), radius, static_cast<I32>(duration * 1000.0f));
	}

	void DebugDrawUtils::DrawCapsule(const Scene& scene, const Vec3& center, const Vec3& color, float radius, float height, const Vec3& rotation, float segaments, float duration)
	{
		const Mat4 rotationMatrix = glm::toMat4(Quat(rotation));
		const Vec4 up = rotationMatrix * Vec4{ 0.0f, 1.0f, 0.0f, 0.0f };
		const Vec4 right = rotationMatrix * Vec4{ 1.0f, 0.0f, 0.0f, 0.0f };
		const Vec4 front = rotationMatrix * Vec4{ 0.0f, 0.0f, -1.0f, 0.0f };
		dd::capsule(scene.GetContextShared()->DebugDrawContext, glm::value_ptr(center), glm::value_ptr(right), glm::value_ptr(up), glm::value_ptr(color), radius, height, segaments, static_cast<I32>(duration * 1000.0f));
		dd::capsule(scene.GetContextShared()->DebugDrawContext, glm::value_ptr(center), glm::value_ptr(front), glm::value_ptr(up), glm::value_ptr(color), radius, height, segaments, static_cast<I32>(duration * 1000.0f));
		dd::circle(scene.GetContextShared()->DebugDrawContext, glm::value_ptr(center + height * 0.5f * Vec3(up)), glm::value_ptr(up), glm::value_ptr(color), radius, segaments, static_cast<I32>(duration * 1000.0f));
		dd::circle(scene.GetContextShared()->DebugDrawContext, glm::value_ptr(center - height * 0.5f * Vec3(up)), glm::value_ptr(up), glm::value_ptr(color), radius, segaments, static_cast<I32>(duration * 1000.0f));
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

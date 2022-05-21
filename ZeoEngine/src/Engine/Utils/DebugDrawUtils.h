#pragma once

#include <glm/glm.hpp>

namespace ZeoEngine {

	class Scene;

	class DebugDrawUtils
	{
	public:
		/** Add a point in 3D space to the debug draw queue. */
		static void DrawPoint(const Ref<Scene>& sceneContext, const glm::vec3& position, const glm::vec3& color, float size = 3.0f, float duration = 0.0f);
		/** Add a 3D line to the debug draw queue. */
		static void DrawLine(const Ref<Scene>& sceneContext, const glm::vec3& from, const glm::vec3& to, const glm::vec3& color, float duration = 0.0f);
		/** Add a wireframe box to the debug draw queue. */
		static void DrawBox(const Ref<Scene>& sceneContext, const glm::vec3& center, const glm::vec3& extent, const glm::vec3& color, const glm::vec3& rotation = glm::vec3(0.0f), float duration = 0.0f);
		/** Add a wireframe circle to the debug draw queue. */
		static void DrawCircle(const Ref<Scene>& sceneContext, const glm::vec3& center, const glm::vec3& planeNormal, const glm::vec3& color, float radius, float segaments = 32, float duration = 0.0f);
		/** Add a wireframe sphere consisting of three perpendicular circles to the debug draw queue. */
		static void DrawSphereBounds(const Ref<Scene>& sceneContext, const glm::vec3& center, const glm::vec3& color, float radius, float segaments = 32, float duration = 0.0f);
		/** Add a wireframe Y-Axis sphere to the debug draw queue. */
		static void DrawSphere(const Ref<Scene>& sceneContext, const glm::vec3& center, const glm::vec3& color, float radius, float duration = 0.0f);
		/**
		 * Add a 3D line with an arrow-like end to the debug draw queue.
		 * @param size - Defines the arrow head size
		 */
		static void DrawArrow(const Ref<Scene>& sceneContext, const glm::vec3& from, const glm::vec3& to, const glm::vec3& color, float size, float duration = 0.0f);
		/**
		 * Add a wireframe cone to the debug draw queue.
		 * The length of the direction vector determines the thickness of the cone.
		 */
		static void DrawCone(const Ref<Scene>& sceneContext, const glm::vec3& apex, const glm::vec3& direction, const glm::vec3& color, float baseRadius, float apexRadius, float duration = 0.0f);
		/**
		 * Add a wireframe plane in 3D space to the debug draw queue.
		 * @param normalVecScale - If not zero, a line depicting the plane normal is also drawn
		 */
		static void DrawPlane(const Ref<Scene>& sceneContext, const glm::vec3& center, const glm::vec3& planeNormal, const glm::vec3& planeColor, const glm::vec3& normalVecColor, float planeScale, float normalVecScale, float duration = 0.0f);
		/**
		 * Add a wireframe frustum pyramid to the debug draw queue.
		 * @param invClipMatrix - inverse(projMatrix * viewMatrix)
		 */
		static void DrawFrustum(const Ref<Scene>& sceneContext, const glm::mat4& invClipMatrix, const glm::vec3& color, float duration = 0.0f);

	};

}

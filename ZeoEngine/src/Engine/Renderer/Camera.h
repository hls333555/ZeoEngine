#pragma once

#include <glm/glm.hpp>

namespace ZeoEngine {

	class Camera
	{
	public:
		Camera() = default;
		Camera(const Mat4& projection)
			: m_Projection(projection) {}

		virtual ~Camera() = default;

		virtual float GetNearClip() const = 0;
		virtual float GetFarClip() const = 0;
		const Mat4& GetProjection() const { return m_Projection; }

	protected:
		Mat4 m_Projection = Mat4(1.0f);

	};

}

#pragma once

#include <glm/glm.hpp>

namespace ZeoEngine {

	class Light
	{
	public:
		const glm::vec4& GetColor() const { return m_Color; }
		void SetColor(const glm::vec4& color) { m_Color = color; }
		float GetIntensity() const { return m_Intensity; }
		void SetIntensity(float intensity) { m_Intensity = intensity; }

		// TODO:
		virtual void Setup() {}

	private:
		glm::vec4 m_Color{ 1.0f };
		float m_Intensity = 1.0f;
	};

	class DirectionalLight : public Light
	{
	public:
		glm::vec3 CalculateDirection(const glm::vec3& rotation) const;
	};

	class PointLight : public Light
	{
	private:

	};

	class SpotLight : public Light
	{
	private:

	};

}

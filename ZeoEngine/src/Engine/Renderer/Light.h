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

		virtual float GetRadius() const { return 0.0f; }
		virtual void SetRadius(float radius) {}

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
	public:
		virtual float GetRadius() const override { return m_Radius; }
		virtual void SetRadius(float radius) override { m_Radius = radius; }

	private:
		float m_Radius = 10.0f;
	};

	class SpotLight : public Light
	{
	private:

	};

}

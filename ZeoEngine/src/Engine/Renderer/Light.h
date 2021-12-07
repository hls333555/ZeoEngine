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
		float GetRange() const { return m_Range; }
		void SetRange(float range) { m_Range  = range; }
		virtual float GetCutoff() const { return 0.0f; }
		virtual void SetCutoff(float cutoff) {}

		glm::vec3 CalculateDirection(const glm::vec3& rotation) const;

	private:
		glm::vec4 m_Color{ 1.0f };
		float m_Intensity = 1.0f;
		float m_Range = 1.0f;
	};

	class DirectionalLight : public Light
	{
	};

	class PointLight : public Light
	{
	};

	class SpotLight : public PointLight
	{
	public:
		virtual float GetCutoff() const override { return m_CutoffAngle; }
		virtual void SetCutoff(float cutoff) override { m_CutoffAngle = cutoff; }

	private:
		/** Half the angle of the spot light cone stored in radians */
		float m_CutoffAngle = glm::radians(30.0f);
	};

}

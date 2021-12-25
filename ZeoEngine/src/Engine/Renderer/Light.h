#pragma once

#include <glm/glm.hpp>

#include "Engine/Renderer/Camera.h"

namespace ZeoEngine {

	class Light
	{
	public:
		enum class ShadowType
		{
			HardShadow = 0,
			PCF,
			PCSS,
		};

		const glm::vec4& GetColor() const { return m_Color; }
		void SetColor(const glm::vec4& color) { m_Color = color; }
		float GetIntensity() const { return m_Intensity; }
		void SetIntensity(float intensity) { m_Intensity = intensity; }
		float GetRange() const { return m_Range; }
		void SetRange(float range) { m_Range  = range; }
		virtual float GetCutoff() const { return 0.0f; }
		virtual void SetCutoff(float cutoff) {}
		bool IsCastShadow() const { return m_bCastShadow; }
		void SetCastShadow(bool bCast) { m_bCastShadow = bCast; }
		ShadowType GetShadowType() const { return m_ShadowType; }
		void SetShadowType(ShadowType type) { m_ShadowType = type; }
		float GetDepthBias() const { return m_DepthBias; }
		void SetDepthBias(float bias) { m_DepthBias = bias; }
		float GetNormalBias() const { return m_NormalBias; }
		void SetNormalBias(float bias) { m_NormalBias = bias; }
		uint32_t GetPcfLevel() const { return m_PcfLevel; }
		void SetPcfLevel(uint32_t level) { m_PcfLevel = level; }
		float GetLightSize() const { return m_LightSize; }
		void SetLightSize(float size) { m_LightSize = size; }
		float GetNearPlane() const { return m_NearPlane; }
		void SetNearPlane(float nearPlane) { m_NearPlane = nearPlane; }

		glm::vec3 CalculateDirection(const glm::vec3& rotation) const;

	private:
		glm::vec4 m_Color{ 1.0f };
		float m_Intensity = 1.0f;
		float m_Range = 1.0f;
		bool m_bCastShadow = false;
		ShadowType m_ShadowType = ShadowType::HardShadow;
		float m_DepthBias = 1.0f;
		float m_NormalBias = 0.1f;
		uint32_t m_PcfLevel = 3;
		float m_LightSize = 0.05f;
		float m_NearPlane = 0.1f;
	};

	class DirectionalLight : public Light, public Camera
	{
	public:
		DirectionalLight();
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

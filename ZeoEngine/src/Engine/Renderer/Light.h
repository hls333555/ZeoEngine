#pragma once

#include <glm/glm.hpp>

namespace ZeoEngine {

	class Scene;

	class Light
	{
		friend struct LightComponentBase;

	public:
		explicit Light(const Ref<Scene>& scene)
			: m_Scene(scene) {}
		virtual ~Light() = default;

		enum class ShadowType
		{
			HardShadow = 0,
			PCF,
			PCSS,
		};

		const Vec4& GetColor() const { return m_Color; }
		void SetColor(const Vec4& color) { m_Color = color; }
		float GetIntensity() const { return m_Intensity; }
		void SetIntensity(float intensity) { m_Intensity = intensity; }
		bool IsCastShadow() const { return m_bCastShadow; }
		void SetCastShadow(bool bCast) { m_bCastShadow = bCast; }
		ShadowType GetShadowType() const { return m_ShadowType; }
		void SetShadowType(ShadowType type) { m_ShadowType = type; }
		float GetDepthBias() const { return m_DepthBias; }
		void SetDepthBias(float bias) { m_DepthBias = bias; }
		float GetNormalBias() const { return m_NormalBias; }
		void SetNormalBias(float bias) { m_NormalBias = bias; }
		float GetLightSize() const { return m_LightSize; }
		void SetLightSize(float size) { m_LightSize = size; }
		float GetFilterSize() const { return m_FilterSize; }
		void SetFilterSize(float size) { m_FilterSize = size; }

		virtual void OnShadowTypeChanged() const {}

		Vec3 CalculateDirection(const Vec3& rotation) const;

	protected:
		Ref<Scene> GetScene() const { return m_Scene.lock(); }

	private:
		Vec4 m_Color{ 1.0f };
		float m_Intensity = 1.0f;
		bool m_bCastShadow = false;
		ShadowType m_ShadowType = ShadowType::HardShadow;
		float m_DepthBias = 0.002f;
		float m_NormalBias = 30.0f;
		float m_FilterSize = 20.0f;
		float m_LightSize = 150.0f;

		Weak<Scene> m_Scene;
	};

	class DirectionalLight : public Light
	{
		friend struct DirectionalLightComponent;

	public:
		using Light::Light;

		virtual void OnShadowTypeChanged() const override;

		U32 GetCascadeCount() const { return m_CascadeCount; }
		void SetCascadeCount(U32 count) { m_CascadeCount = count; }
		float GetCascadeBlendThreshold() const { return m_CascadeBlendThreshold; }
		void SetCascadeBlendThreshold(float threshold) { m_CascadeBlendThreshold = threshold; }
		float GetMaxShadowDistance() const { return m_MaxShadowDistance; }
		void SetMaxShadowDistance(float distance) { m_MaxShadowDistance = distance; }
		float GetCascadeSplitLambda() const { return m_CascadeSplitLambda; }
		void SetGetCascadeSplitLambda(float lambda) { m_CascadeSplitLambda = lambda; }

	private:
		U32 m_CascadeCount = 4;
		float m_CascadeBlendThreshold = 0.5f;
		float m_MaxShadowDistance = 100.0f;
		float m_CascadeSplitLambda = 0.85f;
	};

	class PointLight : public Light
	{
		friend struct PointLightComponent;

	public:
		using Light::Light;

		float GetRange() const { return m_Range; }
		void SetRange(float range) { m_Range = range; }

	private:
		float m_Range = 1.0f;
	};

	class SpotLight : public PointLight
	{
		friend struct SpotLightComponent;

	public:
		using PointLight::PointLight;

		float GetCutoffInRadians() const { return glm::radians(m_CutoffAngle); }
		void SetCutoffAngle(float angle) { m_CutoffAngle = glm::degrees(angle); }

	private:
		/** Half the angle of the spot light cone stored in degrees */
		float m_CutoffAngle = 30.0f;
	};

}

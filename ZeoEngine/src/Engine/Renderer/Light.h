#pragma once

#include <glm/glm.hpp>

#include "Engine/Renderer/RenderPass.h"

namespace ZeoEngine {

	class Scene;
	class ShaderInstance;
	class Camera;

	class Light
	{
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
		float GetRange() const { return m_Range; }
		void SetRange(float range) { m_Range  = range; }
		virtual float GetCutoff() const { return 0.0f; }
		virtual void SetCutoff(float cutoff) {}
		bool IsCastShadow() const { return m_bCastShadow; }
		void SetCastShadow(bool bCast) { m_bCastShadow = bCast; }
		ShadowType GetShadowType() const { return m_ShadowType; }
		virtual void SetShadowType(ShadowType type) { m_ShadowType = type; }
		float GetDepthBias() const { return m_DepthBias; }
		void SetDepthBias(float bias) { m_DepthBias = bias; }
		float GetNormalBias() const { return m_NormalBias; }
		void SetNormalBias(float bias) { m_NormalBias = bias; }
		float GetLightSize() const { return m_LightSize; }
		void SetLightSize(float size) { m_LightSize = size; }
		float GetFilterSize() const { return m_FilterSize; }
		void SetFilterSize(float size) { m_FilterSize = size; }
		virtual U32 GetCascadeCount() const { return 0; }
		virtual void SetCascadeCount(U32 count) {}
		virtual float GetCascadeBlendThreshold() const { return 0.0f; }
		virtual void SetCascadeBlendThreshold(float threshold) {}
		virtual float GetMaxShadowDistance() { return 0.0f; }
		virtual void SetMaxShadowDistance(float distance) {}
		virtual float GetCascadeSplitLambda() { return 0.0f; }
		virtual void SetGetCascadeSplitLambda(float lambda) {}

		Vec3 CalculateDirection(const Vec3& rotation) const;

	protected:
		Ref<Scene> GetScene() const { return m_Scene.lock(); }

	private:
		Vec4 m_Color{ 1.0f };
		float m_Intensity = 1.0f;
		float m_Range = 1.0f;
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
		using Light::Light;

	public:
		virtual void SetShadowType(ShadowType type) override;
		virtual U32 GetCascadeCount() const override { return m_CascadeCount; }
		virtual void SetCascadeCount(U32 count) override { m_CascadeCount = count; }
		virtual float GetCascadeBlendThreshold() const override { return m_CascadeBlendThreshold; }
		virtual void SetCascadeBlendThreshold(float threshold) override { m_CascadeBlendThreshold = threshold; }
		virtual float GetMaxShadowDistance() override { return m_MaxShadowDistance; }
		virtual void SetMaxShadowDistance(float distance) override { m_MaxShadowDistance = distance; }
		virtual float GetCascadeSplitLambda() override { return m_CascadeSplitLambda; }
		virtual void SetGetCascadeSplitLambda(float lambda) override { m_CascadeSplitLambda = lambda; }

	private:
		U32 m_CascadeCount = 4;
		float m_CascadeBlendThreshold = 0.5f;
		float m_MaxShadowDistance =100.0f;
		float m_CascadeSplitLambda = 0.85f;
	};

	class PointLight : public Light
	{
		using Light::Light;
	};

	class SpotLight : public PointLight
	{
		using PointLight::PointLight;

	public:
		virtual float GetCutoff() const override { return m_CutoffAngle; }
		virtual void SetCutoff(float cutoff) override { m_CutoffAngle = cutoff; }

	private:
		/** Half the angle of the spot light cone stored in radians */
		float m_CutoffAngle = glm::radians(30.0f);
	};

}

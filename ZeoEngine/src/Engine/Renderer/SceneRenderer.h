#pragma once

#include <glm/glm.hpp>
#include <entt.hpp>

#include "Engine/Core/Core.h"
#include "Engine/Renderer/BatchRenderer.h"
#include "Engine/Renderer/SceneSettings.h"

namespace ZeoEngine {

	class Camera;
	class EditorCamera;
	class FrameBuffer;
	class RenderGraph;
	class RenderSystemBase;
	class Shader;
	class UniformBuffer;
	class DDRenderInterface;
	class MeshInstance;
	class DirectionalLight;
	class PointLight;
	class SpotLight;

	class SceneRenderer : public std::enable_shared_from_this<SceneRenderer>
	{
		friend class EditorBase;

	public:
		virtual ~SceneRenderer();

		void OnAttach();
		void OnRender();

		void SetupDirectionalLight(const glm::vec3& rotation, const Ref<DirectionalLight>& directionalLight);
		void AddPointLight(const glm::vec3& position, const Ref<PointLight>& pointLight);
		void AddSpotLight(const glm::vec3& position, const glm::vec3& rotation, const Ref<SpotLight>& spotLight);

		void DrawMesh(const glm::mat4& transform, const Ref<MeshInstance>& mesh, int32_t entityID = -1);

		void DrawQuad(const glm::mat4& transform, const glm::vec4& color, int32_t entityID = -1);
		void DrawQuad(const glm::mat4& transform, const Ref<Texture2D>& texture, const glm::vec2& tilingFactor = { 1.0f, 1.0f }, const glm::vec2& uvOffset = { 0.0f, 0.0f }, const glm::vec4& tintColor = glm::vec4(1.0f), int32_t entityID = -1);
		void DrawBillboard(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec2& tilingFactor = { 1.0f, 1.0f }, const glm::vec2& uvOffset = { 0.0f, 0.0f }, const glm::vec4& tintColor = glm::vec4(1.0f), int32_t entityID = -1);

		const Ref<FrameBuffer>& GetFrameBuffer() const { return m_FBO; }
		const RenderGraph& GetRenderGraph() const { return *m_RenderGraph; }
		const Scope<RenderSystemBase>& GetRenderSystem() const { return m_RenderSystem; }

		glm::mat4 GetViewProjectionMatrix() const { return m_CameraBuffer.GetViewProjection(); }

	private:
		virtual Ref<FrameBuffer> CreateFrameBuffer() = 0;
		virtual Scope<RenderGraph> CreateRenderGraph(const Ref<FrameBuffer>& fbo) = 0;
		virtual Scope<RenderSystemBase> CreateRenderSystem() = 0;

		virtual void OnRenderScene() = 0;
	protected:
		virtual void Prepare();
		/** Begin scene for editor. */
		void BeginScene(const EditorCamera& camera);
		/** Begin scene for runtime. */
		void BeginScene(const Camera& camera, const glm::mat4& transform);
		void EndScene();

	private:
		void UpdateCascadeData(const Ref<DirectionalLight>& directionalLight, const glm::vec3& direction);
		void UploadLightData();
		void FlushDebugDraws();

		void OnViewportResize(uint32_t width, uint32_t height);

	private:
		EditorCamera* m_EditorCamera = nullptr;

		Ref<FrameBuffer> m_FBO;
		Scope<RenderGraph> m_RenderGraph;
		Scope<RenderSystemBase> m_RenderSystem;

		Ref<DDRenderInterface> m_Ddri; // TODO: Context? Per-SceneRenderer flush?

		entt::delegate<void(const Ref<FrameBuffer>&)> m_PostSceneRenderDel;

		BatchRenderer m_Batcher;

		struct GlobalData
		{
			glm::vec2 ScreenSize;
		};
		GlobalData m_GlobalBuffer;
		Ref<UniformBuffer> m_GlobalUniformBuffer;

		struct CameraData
		{
			glm::mat4 View;
			glm::mat4 Projection;
			glm::vec3 Position;

			glm::mat4 GetViewProjection() const { return Projection * View; }
		};
		CameraData m_CameraBuffer;
		Ref<UniformBuffer> m_CameraUniformBuffer;
		const Camera* m_ActiveCamera = nullptr;

		struct LightDataBase
		{
			glm::vec4 Color;

			float Intensity;
			int32_t bCastShadow;
			int32_t ShadowType;
			float DepthBias;

			float NormalBias;
			float FilterSize;
			float LightSize;
			float _Padding;

			void Reset()
			{
				Color = glm::vec4{ 0.0f };
				Intensity = 0.0f;
			}
		};

		struct DirectionalLightData : public LightDataBase
		{
			glm::vec3 Direction;
			int32_t CascadeCount;
			float CascadeSplits[SceneSettings::MaxCascades];
			glm::mat4 CascadeReferenceMatrix;
			glm::vec4 CascadeOffsets[SceneSettings::MaxCascades];
			glm::vec4 CascadeScales[SceneSettings::MaxCascades];
			float CascadeBlendThreshold;
			float _Padding, _Padding2, _Padding3;

			void Reset()
			{
				LightDataBase::Reset();
				Direction = glm::vec3{ 0.0f };
			}
		};

		struct PointLightData : public LightDataBase
		{
			glm::vec3 Position;
			float Radius;
		};

		struct SpotLightData : public PointLightData
		{
			glm::vec3 Direction;
			float Cutoff;
		};

		struct LightData
		{
			DirectionalLightData DirectionalLightBuffer;
			PointLightData PointLightBuffer[SceneSettings::MaxPointLights];
			SpotLightData SpotLightBuffer[SceneSettings::MaxSpotLights];
			int32_t NumPointLights = 0;
			int32_t NumSpotLights = 0;

			void Reset()
			{
				DirectionalLightBuffer.Reset();
				NumPointLights = 0;
				NumSpotLights = 0;
			}
		};
		LightData m_LightBuffer;
		Ref<UniformBuffer> m_LightUniformBuffer;

		struct ShadowCameraData
		{
			glm::mat4 ViewProjection[SceneSettings::MaxCascades];
		};
		ShadowCameraData m_ShadowCameraBuffer;
		Ref<UniformBuffer> m_ShadowCameraUniformBuffer;
	};

}

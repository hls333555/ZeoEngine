#pragma once

#include <glm/glm.hpp>
#include <entt.hpp>

#include "Engine/Core/Core.h"
#include "Engine/Core/EngineTypes.h"
#include "Engine/Renderer/BatchRenderer.h"
#include "Engine/Renderer/SceneSettings.h"
#include "Engine/Renderer/RenderGraph.h"

namespace ZeoEngine {

	class Camera;
	class EditorCamera;
	class FrameBuffer;
	class Scene;
	struct SceneContext;
	class RenderGraph;
	class RenderSystemBase;
	class Shader;
	class UniformBuffer;
	class DDRenderInterface;
	class MeshInstance;
	class DirectionalLight;
	class PointLight;
	class SpotLight;
	class RenderDoc;

	class SceneRenderer : public std::enable_shared_from_this<SceneRenderer>
	{
		friend class EditorBase;

	public:
		virtual ~SceneRenderer();

		void OnAttach(const Ref<Scene>& scene);
		void OnRender();

		void UpdateSceneContext(const Ref<Scene>& scene);

		void SetupDirectionalLight(const Vec3& rotation, const Ref<DirectionalLight>& directionalLight);
		void AddPointLight(const Vec3& position, const Ref<PointLight>& pointLight);
		void AddSpotLight(const Vec3& position, const Vec3& rotation, const Ref<SpotLight>& spotLight);

		void DrawMesh(const Mat4& transform, const Ref<MeshInstance>& mesh, I32 entityID = -1);

		void DrawQuad(const Mat4& transform, const Vec4& color, I32 entityID = -1);
		void DrawQuad(const Mat4& transform, const AssetHandle<Texture2D>& texture, const Vec2& tilingFactor = { 1.0f, 1.0f }, const Vec2& uvOffset = { 0.0f, 0.0f }, const Vec4& tintColor = Vec4(1.0f), I32 entityID = -1);
		void DrawBillboard(const Vec3& position, const Vec2& size, const AssetHandle<Texture2D>& texture, const Vec2& tilingFactor = { 1.0f, 1.0f }, const Vec2& uvOffset = { 0.0f, 0.0f }, const Vec4& tintColor = Vec4(1.0f), I32 entityID = -1);

		const Ref<FrameBuffer>& GetFrameBuffer() const { return m_RenderGraph->GetBackFrameBuffer(); }
		const RenderGraph& GetRenderGraph() const { return *m_RenderGraph; }
		RenderGraph& GetRenderGraph() { return *m_RenderGraph; }
		const Scope<RenderSystemBase>& GetRenderSystem() const { return m_RenderSystem; }

		Mat4 GetViewProjectionMatrix() const { return m_CameraBuffer.GetViewProjection(); }

	protected:
		/** Begin scene for editor. */
		void BeginScene(const EditorCamera& camera);
		/** Begin scene for runtime. */
		void BeginScene(const Camera& camera, const Mat4& transform);
		void EndScene();

	private:
		virtual Scope<RenderGraph> CreateRenderGraph() = 0;
		virtual Scope<RenderSystemBase> CreateRenderSystem(const Ref<Scene>& scene) = 0;

		void PrepareScene();
		virtual void OnRenderScene() = 0;
		void FlushScene() const;

		void UpdateCascadeData(const Ref<DirectionalLight>& directionalLight, const Vec3& direction);
		void UploadLightData();

		void OnViewportResize(U32 width, U32 height);

	private:
		EditorCamera* m_EditorCamera = nullptr;

		RenderDoc* m_RenderDocRef = nullptr;

		Ref<SceneContext> m_SceneContext;
		Scope<RenderGraph> m_RenderGraph;
		Scope<RenderSystemBase> m_RenderSystem;

		Ref<DDRenderInterface> m_Ddri;

		entt::delegate<void(const Ref<FrameBuffer>&)> m_PostSceneRenderDel;

		BatchRenderer m_QuadBatcher;

		struct GlobalData
		{
			Vec2 ScreenSize;
		};
		GlobalData m_GlobalBuffer;
		Ref<UniformBuffer> m_GlobalUniformBuffer;

		struct CameraData
		{
			Mat4 View;
			Mat4 Projection;
			Vec3 Position;

			Mat4 GetViewProjection() const { return Projection * View; }
		};
		CameraData m_CameraBuffer;
		Ref<UniformBuffer> m_CameraUniformBuffer;
		const Camera* m_ActiveCamera = nullptr;

		struct LightDataBase
		{
			Vec4 Color;

			float Intensity;
			I32 bCastShadow;
			I32 ShadowType;
			float DepthBias;

			float NormalBias;
			float FilterSize;
			float LightSize;
			float _Padding;

			void Reset()
			{
				Color = Vec4{ 0.0f };
				Intensity = 0.0f;
			}
		};

		struct DirectionalLightData : public LightDataBase
		{
			Vec3 Direction;
			I32 CascadeCount;
			float CascadeSplits[SceneSettings::MaxCascades];
			Mat4 CascadeReferenceMatrix;
			Vec4 CascadeOffsets[SceneSettings::MaxCascades];
			Vec4 CascadeScales[SceneSettings::MaxCascades];
			float CascadeBlendThreshold;
			float _Padding, _Padding2, _Padding3;

			void Reset()
			{
				LightDataBase::Reset();
				Direction = Vec3{ 0.0f };
			}
		};

		struct PointLightData : public LightDataBase
		{
			Vec3 Position;
			float Radius;
		};

		struct SpotLightData : public PointLightData
		{
			Vec3 Direction;
			float Cutoff;
		};

		struct LightData
		{
			DirectionalLightData DirectionalLightBuffer;
			PointLightData PointLightBuffer[SceneSettings::MaxPointLights];
			SpotLightData SpotLightBuffer[SceneSettings::MaxSpotLights];
			I32 NumPointLights = 0;
			I32 NumSpotLights = 0;

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
			Mat4 ViewProjection[SceneSettings::MaxCascades];
		};
		ShadowCameraData m_ShadowCameraBuffer;
		Ref<UniformBuffer> m_ShadowCameraUniformBuffer;
	};

}

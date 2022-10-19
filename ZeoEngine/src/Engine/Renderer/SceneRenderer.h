#pragma once

#include <glm/glm.hpp>
#include <entt.hpp>

#include "Engine/Asset/Asset.h"
#include "Engine/Core/Core.h"
#include "Engine/Core/EngineTypes.h"
#include "Engine/Renderer/BatchRenderer.h"
#include "Engine/Renderer/SceneSettings.h"
#include "Engine/Renderer/RenderGraph.h"

namespace ZeoEngine {

	class Camera;
	class EditorCamera;
	class FrameBuffer;
	class WorldBase;
	class Scene;
	struct SceneContext;
	class RenderGraph;
	class RenderSystemBase;
	class Shader;
	class UniformBuffer;
	class DDRenderInterface;
	class MeshInstance;
	struct DirectionalLightComponent;
	struct PointLightComponent;
	struct SpotLightComponent;
	class RenderDoc;

	class SceneRenderer
	{
	public:
		SceneRenderer();
		virtual ~SceneRenderer();

		virtual void OnAttach(WorldBase* world);
		void OnRender();

		void SetupDirectionalLight(const Vec3& rotation, const DirectionalLightComponent& lightComp);
		void AddPointLight(const Vec3& position, const PointLightComponent& lightComp);
		void AddSpotLight(const Vec3& position, const Vec3& rotation, const SpotLightComponent& lightComp);

		void DrawMesh(const Mat4& transform, const Ref<MeshInstance>& mesh, const std::vector<AssetHandle>& materialAssets, I32 entityID = -1);

		void DrawQuad(const Mat4& transform, const Vec4& color, I32 entityID = -1);
		void DrawQuad(const Mat4& transform, const Ref<Texture2D>& texture, const Vec2& tilingFactor = { 1.0f, 1.0f }, const Vec2& uvOffset = { 0.0f, 0.0f }, const Vec4& tintColor = Vec4(1.0f), I32 entityID = -1);
		void DrawBillboard(const Vec3& position, const Vec2& size, const Ref<Texture2D>& texture, const Vec2& tilingFactor = { 1.0f, 1.0f }, const Vec2& uvOffset = { 0.0f, 0.0f }, const Vec4& tintColor = Vec4(1.0f), I32 entityID = -1);

		Ref<FrameBuffer> GetFrameBuffer() const { return m_RenderGraph->GetBackFrameBuffer(); }
		const RenderGraph& GetRenderGraph() const { return *m_RenderGraph; }
		RenderGraph& GetRenderGraph() { return *m_RenderGraph; }
		const Scope<RenderSystemBase>& GetRenderSystem() const { return m_RenderSystem; }

		Mat4 GetViewProjectionMatrix() const { return m_CameraBuffer.GetViewProjection(); }

		void OnViewportResize(U32 width, U32 height) const;

	protected:
		void BeginScene(const EditorCamera& camera);
		void BeginScene(const Camera& camera, const Mat4& transform);
		void EndScene();

	private:
		virtual Scope<RenderGraph> CreateRenderGraph() = 0;
		virtual Scope<RenderSystemBase> CreateRenderSystem(WorldBase* world) = 0;

		void UpdateSceneContext(const Scene* scene);
		void PrepareScene();
		virtual void OnRenderScene() = 0;
		void FlushScene() const;
		
		void UpdateCascadeData(const DirectionalLightComponent& lightComp, const Vec3& direction);
		void UploadLightData();

	public:
		entt::sink<entt::sigh<void(FrameBuffer&)>> m_PostSceneRender{ m_PostSceneRenderDel };

	private:
		RenderDoc* m_RenderDocRef = nullptr;

		SceneContext* m_SceneContext = nullptr;
		Scope<RenderGraph> m_RenderGraph;
		Scope<RenderSystemBase> m_RenderSystem;
		const Camera* m_ActiveCamera = nullptr;

		Ref<DDRenderInterface> m_Ddri;

		entt::sigh<void(FrameBuffer&)> m_PostSceneRenderDel;

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

		struct LightDataBase
		{
			Vec4 Color;

			float Intensity;
			I32 bCastShadow;
			float DepthBias;
			float NormalBias;

			float FilterSize;
			float LightSize;
			float _Padding;
			float _Padding2;

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

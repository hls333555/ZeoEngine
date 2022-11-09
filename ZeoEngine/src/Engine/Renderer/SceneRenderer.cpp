#include "ZEpch.h"
#include "Engine/Renderer/SceneRenderer.h"

#include "Engine/Renderer/DebugDrawRenderInterface.h"
#include "Engine/Renderer/Buffer.h"
#include "Engine/Renderer/RenderGraph.h"
#include "Engine/Renderer/EditorCamera.h"
#include "Engine/Utils/EngineUtils.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/Mesh.h"
#include "Engine/GameFramework/World.h"
#include "Engine/GameFramework/Components.h"
#include "Engine/GameFramework/RenderSystems.h"

namespace ZeoEngine {

	SceneRenderer::SceneRenderer() = default;

	SceneRenderer::~SceneRenderer()
	{
		m_Ddri->Shutdown();
	}

	void SceneRenderer::OnAttach(WorldBase* world)
	{
		m_RenderDocRef = &Application::Get().GetRenderDoc();

		m_QuadBatcher.Init();

		m_Ddri = DDRenderInterface::Create(this);
		m_RenderGraph = CreateRenderGraph();
		m_RenderGraph->Init();
		m_RenderSystem = CreateRenderSystem(world);

		UpdateSceneContext(world->GetActiveScene().get());
		world->m_OnActiveSceneChanged.connect<&SceneRenderer::UpdateSceneContext>(this);

		m_GlobalUniformBuffer = UniformBuffer::Create(sizeof(GlobalData), UniformBufferBinding::Global);
		m_CameraUniformBuffer = UniformBuffer::Create(sizeof(CameraData), UniformBufferBinding::Camera);
		m_LightUniformBuffer = UniformBuffer::Create(sizeof(LightData), UniformBufferBinding::Light);
		m_ShadowCameraUniformBuffer = UniformBuffer::Create(sizeof(ShadowCameraData), UniformBufferBinding::ShadowCamera);
	}

	void SceneRenderer::OnRender()
	{
		ZE_PROFILE_FUNC();

		m_RenderDocRef->StartFrameCapture();
		m_RenderGraph->Start();
		{
			PrepareScene();
			OnRenderScene();
			m_PostSceneRenderDel.publish(*GetFrameBuffer());
		}
		m_RenderGraph->Stop();
		m_RenderDocRef->StopFrameCapture();
	}

	void SceneRenderer::UpdateSceneContext(const Scene* scene)
	{
		m_SceneContext = scene->GetContextShared();
		// Recreate ddri context when a new scene is created
		m_Ddri->Init(m_SceneContext);
	}

	void SceneRenderer::PrepareScene()
	{
		m_GlobalUniformBuffer->Bind();
		m_CameraUniformBuffer->Bind();
		m_LightUniformBuffer->Bind();
		m_ShadowCameraUniformBuffer->Bind();

		const auto fbo = GetFrameBuffer();
		m_GlobalBuffer.ScreenSize = { fbo->GetSpec().Width, fbo->GetSpec().Height };
		m_GlobalUniformBuffer->SetData(&m_GlobalBuffer);

		m_LightBuffer.Reset();
		m_LightUniformBuffer->SetData(&m_LightBuffer);

		m_QuadBatcher.StartBatch();

		m_ActiveCamera = nullptr;
	}

	void SceneRenderer::BeginScene(const EditorCamera& camera)
	{
		m_CameraBuffer.View = camera.GetViewMatrix();
		m_CameraBuffer.Projection = camera.GetProjection();
		m_CameraBuffer.Position = camera.GetPosition();
		m_CameraUniformBuffer->SetData(&m_CameraBuffer);
		m_ActiveCamera = &camera;
	}

	void SceneRenderer::BeginScene(const Camera& camera, const Mat4& transform)
	{
		m_CameraBuffer.View = glm::inverse(transform);
		m_CameraBuffer.Projection = camera.GetProjection();
		m_CameraBuffer.Position = Math::GetTranslationFromTransform(transform);
		m_CameraUniformBuffer->SetData(&m_CameraBuffer);
		m_ActiveCamera = &camera;
	}

	void SceneRenderer::EndScene()
	{
		UploadLightData();
		m_RenderGraph->Execute();
		FlushScene();
	}

	void SceneRenderer::FlushScene() const
	{
		m_QuadBatcher.FlushBatch();
		m_Ddri->Flush(EngineUtils::GetTimeInSeconds() * 1000.0f);
	}

	void SceneRenderer::OnViewportResize(U32 width, U32 height) const
	{
		m_Ddri->UpdateViewportSize(width, height);
		m_RenderGraph->OnViewportResize(width, height);
	}

	void SceneRenderer::SetupDirectionalLight(const Vec3& rotation, const DirectionalLightComponent& lightComp)
	{
		m_LightBuffer.DirectionalLightBuffer.Color = lightComp.Color;
		m_LightBuffer.DirectionalLightBuffer.Intensity = lightComp.Intensity;
		const auto direction = Math::GetForwardVector(rotation);
		m_LightBuffer.DirectionalLightBuffer.Direction = direction;
		m_LightBuffer.DirectionalLightBuffer.bCastShadow = lightComp.bCastShadow;
		m_LightBuffer.DirectionalLightBuffer.DepthBias = lightComp.DepthBias;
		m_LightBuffer.DirectionalLightBuffer.NormalBias = lightComp.NormalBias;
		m_LightBuffer.DirectionalLightBuffer.FilterSize = lightComp.FilterSize;
		m_LightBuffer.DirectionalLightBuffer.LightSize = lightComp.LightSize;
		m_LightBuffer.DirectionalLightBuffer.CascadeCount = lightComp.CascadeCount;
		m_LightBuffer.DirectionalLightBuffer.CascadeBlendThreshold = lightComp.CascadeBlendThreshold;

		// TODO: No need update every frame
		UpdateCascadeData(lightComp, direction);
	}

	void SceneRenderer::AddPointLight(const Vec3& position, const PointLightComponent& lightComp)
	{
		m_LightBuffer.PointLightBuffer[m_LightBuffer.NumPointLights].Color = lightComp.Color;
		m_LightBuffer.PointLightBuffer[m_LightBuffer.NumPointLights].Intensity = lightComp.Intensity;
		m_LightBuffer.PointLightBuffer[m_LightBuffer.NumPointLights].Position = position;
		m_LightBuffer.PointLightBuffer[m_LightBuffer.NumPointLights].Radius = lightComp.Range;
		m_LightBuffer.PointLightBuffer[m_LightBuffer.NumPointLights].bCastShadow = lightComp.bCastShadow;
		m_LightBuffer.PointLightBuffer[m_LightBuffer.NumPointLights].DepthBias = lightComp.DepthBias;
		m_LightBuffer.PointLightBuffer[m_LightBuffer.NumPointLights].NormalBias = lightComp.NormalBias;
		m_LightBuffer.PointLightBuffer[m_LightBuffer.NumPointLights].LightSize = lightComp.LightSize;
		++m_LightBuffer.NumPointLights;
	}

	void SceneRenderer::AddSpotLight(const Vec3& position, const Vec3& rotation, const SpotLightComponent& lightComp)
	{
		m_LightBuffer.SpotLightBuffer[m_LightBuffer.NumSpotLights].Color = lightComp.Color;
		m_LightBuffer.SpotLightBuffer[m_LightBuffer.NumSpotLights].Intensity = lightComp.Intensity;
		m_LightBuffer.SpotLightBuffer[m_LightBuffer.NumSpotLights].Position = position;
		m_LightBuffer.SpotLightBuffer[m_LightBuffer.NumSpotLights].Radius = lightComp.Range;
		m_LightBuffer.SpotLightBuffer[m_LightBuffer.NumSpotLights].Direction = Math::GetForwardVector(rotation);
		m_LightBuffer.SpotLightBuffer[m_LightBuffer.NumSpotLights].Cutoff = cos(lightComp.GetCutoffInRadians());
		m_LightBuffer.SpotLightBuffer[m_LightBuffer.NumSpotLights].bCastShadow = lightComp.bCastShadow;
		m_LightBuffer.SpotLightBuffer[m_LightBuffer.NumSpotLights].DepthBias = lightComp.DepthBias;
		m_LightBuffer.SpotLightBuffer[m_LightBuffer.NumSpotLights].LightSize = lightComp.LightSize;
		++m_LightBuffer.NumSpotLights;
	}

	void SceneRenderer::UpdateCascadeData(const DirectionalLightComponent& lightComp, const Vec3& direction)
	{
		float shadowMapSize = SceneSettings::ShadowMapResolution;

		float nearClip = m_ActiveCamera->GetNearClip();
		float farClip = m_ActiveCamera->GetFarClip();
		float clipRange = farClip - nearClip;

		float cascadeSplits[SceneSettings::MaxCascades];
		{
			float minZ = nearClip;
			float maxZ = glm::clamp(lightComp.MaxShadowDistance, nearClip, farClip);

			float range = maxZ - minZ;
			float ratio = maxZ / minZ;

			float cascadeSplitLambda = lightComp.CascadeSplitLambda;
			U32 cascadeCount = lightComp.CascadeCount;

			// Calculate split depths based on view camera frustum
			// Based on method presented in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
			for (U32 cascadeIndex = 0; cascadeIndex < cascadeCount; ++cascadeIndex)
			{
				float p = (cascadeIndex + 1) / static_cast<float>(cascadeCount);
				float log = minZ * std::pow(ratio, p);
				float uniform = minZ + range * p;
				float d = cascadeSplitLambda * (log - uniform) + uniform;
				cascadeSplits[cascadeIndex] = (d - nearClip) / clipRange;
			}

			// Manually set cascades here
			// cascadeSplits[0] = 0.05f;
			// cascadeSplits[1] = 0.15f;
			// cascadeSplits[2] = 0.3f;
			// cascadeSplits[3] = 1.0f;
		}

		Mat4 invCameraMatrix = glm::inverse(m_CameraBuffer.GetViewProjection());

		// Calculate orthographic projection matrix for each cascade
		for (U32 i = 0; i < lightComp.CascadeCount; ++i)
		{
			float lastSplitDist = i == 0 ? 0.0f : cascadeSplits[i - 1];
			float splitDist = cascadeSplits[i];

			// Get the 8 points of the view frustum in world space
			Vec3 frustumCorners[8] = {
				Vec3(-1.0f,  1.0f, -1.0f),
				Vec3(1.0f,  1.0f, -1.0f),
				Vec3(1.0f, -1.0f, -1.0f),
				Vec3(-1.0f, -1.0f, -1.0f),
				Vec3(-1.0f,  1.0f,  1.0f),
				Vec3(1.0f,  1.0f,  1.0f),
				Vec3(1.0f, -1.0f,  1.0f),
				Vec3(-1.0f, -1.0f,  1.0f),
			};

			// Project frustum corners into world space
			for (auto& corner : frustumCorners)
			{
				Vec4 invCorner = invCameraMatrix * Vec4(corner, 1.0f);
				corner = invCorner / invCorner.w;
			}

			// Get the corners of the current cascade slice of the view frustum
			for (U32 i = 0; i < 4; ++i)
			{
				Vec3 cornerRay = frustumCorners[i + 4] - frustumCorners[i];
				frustumCorners[i + 4] = frustumCorners[i] + (cornerRay * splitDist);
				frustumCorners[i] = frustumCorners[i] + (cornerRay * lastSplitDist);
			}

			// Get frustum center
			Vec3 frustumCenter = Vec3(0.0f);
			for (const auto& corner : frustumCorners)
			{
				frustumCenter += corner;
			}
			frustumCenter /= 8.0f;

			// Calculate the radius of a bounding sphere surrounding the frustum corners
			float radius = 0.0f;
			for (const auto& corner : frustumCorners)
			{
				float distance = glm::length(corner - frustumCenter);
				radius = glm::max(radius, distance);
			}
			radius = std::ceil(radius * 16.0f) / 16.0f;

			Vec3 maxExtents = Vec3(radius);
			Vec3 minExtents = -maxExtents;

			Mat4 lightViewMatrix = glm::lookAt(frustumCenter - direction * -minExtents.z, frustumCenter, Vec3(0.0f, 1.0f, 0.0f));
			float zNear = 0.0f;
			float zFar = maxExtents.z - minExtents.z;
			Mat4 lightOrthoMatrix = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, zNear, zFar);

			// Create the rounding matrix, by projecting the world-space origin and determining the fractional offset in texel space,
			// which ensures that shadow edges do not shimmer
			Mat4 shadowMatrix = lightOrthoMatrix * lightViewMatrix;
			Vec4 shadowOrigin(0.0f, 0.0f, 0.0f, 1.0f);
			shadowOrigin = shadowMatrix * shadowOrigin;
			shadowOrigin = shadowOrigin * shadowMapSize / 2.0f;

			Vec4 roundedOrigin = glm::round(shadowOrigin);
			Vec4 offset = roundedOrigin - shadowOrigin;
			offset = Vec4(Vec2(offset * 2.0f / shadowMapSize), 0.0f, 0.0f);

			lightOrthoMatrix[3] += offset;

			// Store the split distance in terms of view space depth
			m_LightBuffer.DirectionalLightBuffer.CascadeSplits[i] = (nearClip + splitDist * clipRange) * -1.0f;
			m_ShadowCameraBuffer.ViewProjection[i] = lightOrthoMatrix * lightViewMatrix;

			const Mat4 texScaleBias(
				0.5f, 0.0f, 0.0f, 0.0f,
				0.0f, 0.5f, 0.0f, 0.0f,
				0.0f, 0.0f, 0.5f, 0.0f,
				0.5f, 0.5f, 0.5f, 1.0f
			);

			// Create reference matrix from 1st cascade
			m_LightBuffer.DirectionalLightBuffer.CascadeReferenceMatrix = texScaleBias * m_ShadowCameraBuffer.ViewProjection[0];
			// Determine scale and offset for each cascade
			const Mat4 invShadowMatrix = glm::inverse(texScaleBias * m_ShadowCameraBuffer.ViewProjection[i]);
			Vec4 zeroCorner = m_LightBuffer.DirectionalLightBuffer.CascadeReferenceMatrix * invShadowMatrix * Vec4(0, 0, 0, 1);
			Vec4 oneCorner = m_LightBuffer.DirectionalLightBuffer.CascadeReferenceMatrix * invShadowMatrix * Vec4(1, 1, 1, 1);

			m_LightBuffer.DirectionalLightBuffer.CascadeOffsets[i] = Vec4(Vec3(-zeroCorner), 0.0f);
			m_LightBuffer.DirectionalLightBuffer.CascadeScales[i] = Vec4(Vec3(1.0f) / Vec3(oneCorner - zeroCorner), 1.0f);
		}
	}

	void SceneRenderer::UploadLightData()
	{
		m_LightUniformBuffer->SetData(&m_LightBuffer);
		m_ShadowCameraUniformBuffer->SetData(&m_ShadowCameraBuffer);
	}

	void SceneRenderer::DrawMesh(const Mat4& transform, const Ref<MeshInstance>& mesh, const std::vector<AssetHandle>& materialAssets, I32 entityID)
	{
		if (!mesh) return;

		mesh->Submit(transform, materialAssets, entityID);
		Renderer::GetStats().MeshVertexCount += mesh->GetMesh()->GetVertexCount();
	}

	void SceneRenderer::DrawQuad(const Mat4& transform, const Vec4& color, I32 entityID)
	{
		m_QuadBatcher.DrawQuad(transform, color, entityID);
	}

	void SceneRenderer::DrawQuad(const Mat4& transform, const Ref<Texture2D>& texture, const Vec2& tilingFactor, const Vec2& uvOffset, const Vec4& tintColor, I32 entityID)
	{
		m_QuadBatcher.DrawQuad(transform, texture, tilingFactor, uvOffset, tintColor, entityID);
	}

	void SceneRenderer::DrawBillboard(const Vec3& position, const Vec2& size, const Ref<Texture2D>& texture, const Vec2& tilingFactor, const Vec2& uvOffset, const Vec4& tintColor, I32 entityID)
	{
		Mat4 lookAtMatrix = glm::lookAt(position, m_CameraBuffer.Position, { 0.0f, 1.0f, 0.0f });
		Mat4 transform = glm::inverse(lookAtMatrix) *
			glm::scale(Mat4(1.0f), { size.x, size.y, 1.0f });
		Vec4 color = tintColor;
		color.a = 1.0f;
		m_QuadBatcher.DrawQuad(transform, texture, tilingFactor, uvOffset, color, entityID);
	}

}

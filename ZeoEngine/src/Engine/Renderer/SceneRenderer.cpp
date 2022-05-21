#include "ZEpch.h"
#include "Engine/Renderer/SceneRenderer.h"

#include "Engine/Renderer/DebugDrawRenderInterface.h"
#include "Engine/Renderer/Buffer.h"
#include "Engine/Renderer/RenderGraph.h"
#include "Engine/GameFramework/Systems.h"
#include "Engine/Renderer/EditorCamera.h"
#include "Engine/Utils/EngineUtils.h"
#include "Engine/Renderer/RenderCommand.h"
#include "Engine/Renderer/Renderer.h"

namespace ZeoEngine {
	
	SceneRenderer::~SceneRenderer()
	{
		DDRenderInterface::Shutdown(m_SceneContext);
	}

	void SceneRenderer::OnAttach(const Ref<Scene>& scene)
	{
		m_Batcher.Init();

		m_Ddri = DDRenderInterface::Create(shared_from_this());
		UpdateSceneContext(scene);

		m_RenderGraph = CreateRenderGraph();
		m_RenderGraph->Init();
		m_RenderSystem = CreateRenderSystem();
		m_RenderSystem->UpdateScene(scene);

		m_GlobalUniformBuffer = UniformBuffer::Create(sizeof(GlobalData), static_cast<uint32_t>(UniformBufferBinding::Global));
		m_CameraUniformBuffer = UniformBuffer::Create(sizeof(CameraData), static_cast<uint32_t>(UniformBufferBinding::Camera));
		m_LightUniformBuffer = UniformBuffer::Create(sizeof(LightData), static_cast<uint32_t>(UniformBufferBinding::Light));
		m_ShadowCameraUniformBuffer = UniformBuffer::Create(sizeof(ShadowCameraData), static_cast<uint32_t>(UniformBufferBinding::ShadowCamera));
	}

	void SceneRenderer::OnRender()
	{
		m_RenderGraph->Start();
		{
			Prepare();
			OnRenderScene();
			m_PostSceneRenderDel(GetFrameBuffer());
		}
		m_RenderGraph->Stop();
	}

	void SceneRenderer::UpdateSceneContext(const Ref<Scene>& scene)
	{
		m_SceneContext = scene->GetContext();
		// Recreate ddri context when a new scene is created
		m_Ddri->Init(m_SceneContext);
	}

	void SceneRenderer::BeginScene(const EditorCamera& camera)
	{
		const auto& fbo = GetFrameBuffer();
		m_GlobalBuffer.ScreenSize = { fbo->GetSpec().Width, fbo->GetSpec().Height };
		m_GlobalUniformBuffer->SetData(&m_GlobalBuffer);

		m_CameraBuffer.View = camera.GetViewMatrix();
		m_CameraBuffer.Projection = camera.GetProjection();
		m_CameraBuffer.Position = camera.GetPosition();
		m_CameraUniformBuffer->SetData(&m_CameraBuffer);
		m_ActiveCamera = &camera;

		m_RenderGraph->ToggleRenderPassActive("Grid", true);
	}

	void SceneRenderer::BeginScene(const Camera& camera, const glm::mat4& transform)
	{
		const auto& fbo = GetFrameBuffer();
		m_GlobalBuffer.ScreenSize = { fbo->GetSpec().Width, fbo->GetSpec().Height };
		m_GlobalUniformBuffer->SetData(&m_GlobalBuffer);

		m_CameraBuffer.View = glm::inverse(transform);
		m_CameraBuffer.Projection = camera.GetProjection();
		// TODO:
		//m_CameraBuffer.Position = camera.GetPosition();
		m_CameraUniformBuffer->SetData(&m_CameraBuffer);
		m_ActiveCamera = &camera;

		m_RenderGraph->ToggleRenderPassActive("Grid", false);
	}

	void SceneRenderer::EndScene()
	{
		m_ActiveCamera = nullptr;
		UploadLightData();
		m_RenderGraph->Execute();
		// TODO:
		{
			RenderCommand::ToggleDepthWriting(true);
			// Quads are drawn at last
			m_Batcher.FlushBatch();
			DDRenderInterface::Flush(m_SceneContext, EngineUtils::GetTimeInSeconds() * 1000.0f);
		}
	}

	void SceneRenderer::Prepare()
	{
		m_LightBuffer.Reset();
		m_LightUniformBuffer->SetData(&m_LightBuffer);

		m_Batcher.StartBatch();
	}

	void SceneRenderer::OnViewportResize(uint32_t width, uint32_t height)
	{
		m_Ddri->UpdateViewportSize(width, height);
	}

	void SceneRenderer::SetupDirectionalLight(const glm::vec3& rotation, const Ref<DirectionalLight>& directionalLight)
	{
		m_LightBuffer.DirectionalLightBuffer.Color = directionalLight->GetColor();
		m_LightBuffer.DirectionalLightBuffer.Intensity = directionalLight->GetIntensity();
		const auto direction = directionalLight->CalculateDirection(rotation);
		m_LightBuffer.DirectionalLightBuffer.Direction = direction;
		m_LightBuffer.DirectionalLightBuffer.bCastShadow = directionalLight->IsCastShadow();
		m_LightBuffer.DirectionalLightBuffer.ShadowType = static_cast<int32_t>(directionalLight->GetShadowType());
		m_LightBuffer.DirectionalLightBuffer.DepthBias = directionalLight->GetDepthBias();
		m_LightBuffer.DirectionalLightBuffer.NormalBias = directionalLight->GetNormalBias();
		m_LightBuffer.DirectionalLightBuffer.FilterSize = directionalLight->GetFilterSize();
		m_LightBuffer.DirectionalLightBuffer.LightSize = directionalLight->GetLightSize();
		m_LightBuffer.DirectionalLightBuffer.CascadeCount = directionalLight->GetCascadeCount();
		m_LightBuffer.DirectionalLightBuffer.CascadeBlendThreshold = directionalLight->GetCascadeBlendThreshold();

		// TODO: No need update every frame
		UpdateCascadeData(directionalLight, direction);
	}

	void SceneRenderer::AddPointLight(const glm::vec3& position, const Ref<PointLight>& pointLight)
	{
		m_LightBuffer.PointLightBuffer[m_LightBuffer.NumPointLights].Color = pointLight->GetColor();
		m_LightBuffer.PointLightBuffer[m_LightBuffer.NumPointLights].Intensity = pointLight->GetIntensity();
		m_LightBuffer.PointLightBuffer[m_LightBuffer.NumPointLights].Position = position;
		m_LightBuffer.PointLightBuffer[m_LightBuffer.NumPointLights].Radius = pointLight->GetRange();
		m_LightBuffer.PointLightBuffer[m_LightBuffer.NumPointLights].bCastShadow = pointLight->IsCastShadow();
		m_LightBuffer.PointLightBuffer[m_LightBuffer.NumPointLights].ShadowType = static_cast<int32_t>(pointLight->GetShadowType());
		m_LightBuffer.PointLightBuffer[m_LightBuffer.NumPointLights].DepthBias = pointLight->GetDepthBias();
		m_LightBuffer.PointLightBuffer[m_LightBuffer.NumPointLights].NormalBias = pointLight->GetNormalBias();
		m_LightBuffer.PointLightBuffer[m_LightBuffer.NumPointLights].LightSize = pointLight->GetLightSize();
		++m_LightBuffer.NumPointLights;
	}

	void SceneRenderer::AddSpotLight(const glm::vec3& position, const glm::vec3& rotation, const Ref<SpotLight>& spotLight)
	{
		m_LightBuffer.SpotLightBuffer[m_LightBuffer.NumSpotLights].Color = spotLight->GetColor();
		m_LightBuffer.SpotLightBuffer[m_LightBuffer.NumSpotLights].Intensity = spotLight->GetIntensity();
		m_LightBuffer.SpotLightBuffer[m_LightBuffer.NumSpotLights].Position = position;
		m_LightBuffer.SpotLightBuffer[m_LightBuffer.NumSpotLights].Radius = spotLight->GetRange();
		m_LightBuffer.SpotLightBuffer[m_LightBuffer.NumSpotLights].Direction = spotLight->CalculateDirection(rotation);
		m_LightBuffer.SpotLightBuffer[m_LightBuffer.NumSpotLights].Cutoff = cos(spotLight->GetCutoff());
		m_LightBuffer.SpotLightBuffer[m_LightBuffer.NumSpotLights].bCastShadow = spotLight->IsCastShadow();
		m_LightBuffer.SpotLightBuffer[m_LightBuffer.NumSpotLights].ShadowType = static_cast<int32_t>(spotLight->GetShadowType());
		m_LightBuffer.SpotLightBuffer[m_LightBuffer.NumSpotLights].DepthBias = spotLight->GetDepthBias();
		m_LightBuffer.SpotLightBuffer[m_LightBuffer.NumSpotLights].LightSize = spotLight->GetLightSize();
		++m_LightBuffer.NumSpotLights;
	}

	void SceneRenderer::UpdateCascadeData(const Ref<DirectionalLight>& directionalLight, const glm::vec3& direction)
	{
		float shadowMapSize = SceneSettings::ShadowMapResolution;

		float nearClip = m_ActiveCamera->GetNearClip();
		float farClip = m_ActiveCamera->GetFarClip();
		float clipRange = farClip - nearClip;

		float cascadeSplits[SceneSettings::MaxCascades];
		{
			float minZ = nearClip;
			float maxZ = glm::clamp(directionalLight->GetMaxShadowDistance(), nearClip, farClip);

			float range = maxZ - minZ;
			float ratio = maxZ / minZ;

			float cascadeSplitLambda = directionalLight->GetCascadeSplitLambda();
			uint32_t cascadeCount = directionalLight->GetCascadeCount();

			// Calculate split depths based on view camera frustum
			// Based on method presented in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
			for (uint32_t cascadeIndex = 0; cascadeIndex < cascadeCount; ++cascadeIndex)
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

		glm::mat4 invCameraMatrix = glm::inverse(m_CameraBuffer.GetViewProjection());

		// Calculate orthographic projection matrix for each cascade
		for (uint32_t i = 0; i < directionalLight->GetCascadeCount(); ++i)
		{
			float lastSplitDist = i == 0 ? 0.0f : cascadeSplits[i - 1];
			float splitDist = cascadeSplits[i];

			// Get the 8 points of the view frustum in world space
			glm::vec3 frustumCorners[8] = {
				glm::vec3(-1.0f,  1.0f, -1.0f),
				glm::vec3(1.0f,  1.0f, -1.0f),
				glm::vec3(1.0f, -1.0f, -1.0f),
				glm::vec3(-1.0f, -1.0f, -1.0f),
				glm::vec3(-1.0f,  1.0f,  1.0f),
				glm::vec3(1.0f,  1.0f,  1.0f),
				glm::vec3(1.0f, -1.0f,  1.0f),
				glm::vec3(-1.0f, -1.0f,  1.0f),
			};

			// Project frustum corners into world space
			for (auto& corner : frustumCorners)
			{
				glm::vec4 invCorner = invCameraMatrix * glm::vec4(corner, 1.0f);
				corner = invCorner / invCorner.w;
			}

			// Get the corners of the current cascade slice of the view frustum
			for (uint32_t i = 0; i < 4; ++i)
			{
				glm::vec3 cornerRay = frustumCorners[i + 4] - frustumCorners[i];
				frustumCorners[i + 4] = frustumCorners[i] + (cornerRay * splitDist);
				frustumCorners[i] = frustumCorners[i] + (cornerRay * lastSplitDist);
			}

			// Get frustum center
			glm::vec3 frustumCenter = glm::vec3(0.0f);
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

			glm::vec3 maxExtents = glm::vec3(radius);
			glm::vec3 minExtents = -maxExtents;

			glm::mat4 lightViewMatrix = glm::lookAt(frustumCenter - direction * -minExtents.z, frustumCenter, glm::vec3(0.0f, 1.0f, 0.0f));
			float zNear = 0.0f;
			float zFar = maxExtents.z - minExtents.z;
			glm::mat4 lightOrthoMatrix = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, zNear, zFar);

			// Create the rounding matrix, by projecting the world-space origin and determining the fractional offset in texel space,
			// which ensures that shadow edges do not shimmer
			glm::mat4 shadowMatrix = lightOrthoMatrix * lightViewMatrix;
			glm::vec4 shadowOrigin(0.0f, 0.0f, 0.0f, 1.0f);
			shadowOrigin = shadowMatrix * shadowOrigin;
			shadowOrigin = shadowOrigin * shadowMapSize / 2.0f;

			glm::vec4 roundedOrigin = glm::round(shadowOrigin);
			glm::vec4 offset = roundedOrigin - shadowOrigin;
			offset = glm::vec4(glm::vec2(offset * 2.0f / shadowMapSize), 0.0f, 0.0f);

			lightOrthoMatrix[3] += offset;

			// Store the split distance in terms of view space depth
			m_LightBuffer.DirectionalLightBuffer.CascadeSplits[i] = (nearClip + splitDist * clipRange) * -1.0f;
			m_ShadowCameraBuffer.ViewProjection[i] = lightOrthoMatrix * lightViewMatrix;

			const glm::mat4 texScaleBias(
				0.5f, 0.0f, 0.0f, 0.0f,
				0.0f, 0.5f, 0.0f, 0.0f,
				0.0f, 0.0f, 0.5f, 0.0f,
				0.5f, 0.5f, 0.5f, 1.0f
			);

			// Create reference matrix from 1st cascade
			m_LightBuffer.DirectionalLightBuffer.CascadeReferenceMatrix = texScaleBias * m_ShadowCameraBuffer.ViewProjection[0];
			// Determine scale and offset for each cascade
			const glm::mat4 invShadowMatrix = glm::inverse(texScaleBias * m_ShadowCameraBuffer.ViewProjection[i]);
			glm::vec4 zeroCorner = m_LightBuffer.DirectionalLightBuffer.CascadeReferenceMatrix * invShadowMatrix * glm::vec4(0, 0, 0, 1);
			glm::vec4 oneCorner = m_LightBuffer.DirectionalLightBuffer.CascadeReferenceMatrix * invShadowMatrix * glm::vec4(1, 1, 1, 1);

			m_LightBuffer.DirectionalLightBuffer.CascadeOffsets[i] = glm::vec4(glm::vec3(-zeroCorner), 0.0f);
			m_LightBuffer.DirectionalLightBuffer.CascadeScales[i] = glm::vec4(glm::vec3(1.0f) / glm::vec3(oneCorner - zeroCorner), 1.0f);
		}
	}

	void SceneRenderer::UploadLightData()
	{
		m_LightUniformBuffer->SetData(&m_LightBuffer);
		m_ShadowCameraUniformBuffer->SetData(&m_ShadowCameraBuffer);
	}

	void SceneRenderer::DrawMesh(const glm::mat4& transform, const Ref<MeshInstance>& mesh, int32_t entityID)
	{
		if (!mesh) return;

		mesh->Submit(transform, entityID);
		Renderer::GetStats().MeshVertexCount += mesh->GetMesh()->GetVertexCount();
	}

	void SceneRenderer::DrawQuad(const glm::mat4& transform, const glm::vec4& color, int32_t entityID)
	{
		m_Batcher.DrawQuad(transform, color, entityID);
	}

	void SceneRenderer::DrawQuad(const glm::mat4& transform, const AssetHandle<Texture2D>& texture, const glm::vec2& tilingFactor, const glm::vec2& uvOffset, const glm::vec4& tintColor, int32_t entityID)
	{
		m_Batcher.DrawQuad(transform, texture, tilingFactor, uvOffset, tintColor, entityID);
	}

	void SceneRenderer::DrawBillboard(const glm::vec3& position, const glm::vec2& size, const AssetHandle<Texture2D>& texture, const glm::vec2& tilingFactor, const glm::vec2& uvOffset, const glm::vec4& tintColor, int32_t entityID)
	{
		glm::mat4 lookAtMatrix = glm::lookAt(position, m_CameraBuffer.Position, { 0.0f, 1.0f, 0.0f });
		glm::mat4 transform = glm::inverse(lookAtMatrix) *
			glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
		glm::vec4 color = tintColor;
		color.a = 1.0f;
		m_Batcher.DrawQuad(transform, texture, tilingFactor, uvOffset, color, entityID);
	}

}

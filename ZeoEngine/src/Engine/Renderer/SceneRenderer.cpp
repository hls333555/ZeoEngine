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
		DDRenderInterface::Shutdown();
	}

	void SceneRenderer::OnAttach()
	{
		m_Batcher.Init();

		m_Ddri = DDRenderInterface::Create(shared_from_this());
		// Init debug draw render interface
		DDRenderInterface::Init(m_Ddri);

		m_FBO = CreateFrameBuffer();
		ZE_CORE_ASSERT(m_FBO);
		m_RenderGraph = CreateRenderGraph(m_FBO);
		m_RenderSystem = CreateRenderSystem();

		m_CameraUniformBuffer = UniformBuffer::Create(sizeof(CameraData), 0);

		m_GridShader = Shader::Create("assets/editor/shaders/Grid.glsl");
		m_GridUniformBuffer = UniformBuffer::Create(sizeof(GridData), 1);
		m_GridUniformBuffer->SetData(&m_GridBuffer);

		m_LightUniformBuffer = UniformBuffer::Create(sizeof(LightData), 2);
	}

	void SceneRenderer::OnRender()
	{
		Renderer::SetActiveRenderGraph(m_RenderGraph.get());
		m_FBO->Bind();
		{
			Prepare();
			OnRenderScene();
			m_PostSceneRenderDel(m_FBO);
			m_RenderGraph->Reset();
			FlushDebugDraws();
		}
		m_FBO->Unbind();
	}

	void SceneRenderer::BeginScene(const EditorCamera& camera, bool bDrawGrid)
	{
		m_CameraBuffer.View = camera.GetViewMatrix();
		m_CameraBuffer.Projection = camera.GetProjection();
		m_CameraBuffer.Position = camera.GetPosition();
		m_CameraUniformBuffer->SetData(&m_CameraBuffer);

		m_bDrawGrid = bDrawGrid;
	}

	void SceneRenderer::BeginScene(const Camera& camera, const glm::mat4& transform)
	{
		m_CameraBuffer.View = glm::inverse(transform);
		m_CameraBuffer.Projection = camera.GetProjection();
		m_CameraUniformBuffer->SetData(&m_CameraBuffer);
	}

	void SceneRenderer::EndScene()
	{
		UploadLightData();
		m_RenderGraph->Execute();
		RenderCommand::ToggleFaceCulling(false);
		RenderCommand::ToggleDepthWriting(false);
		{
			// Translucent grid
			RenderGrid();
		}
		RenderCommand::ToggleDepthWriting(true);
		{
			// Quads are drawn at last
			m_Batcher.FlushBatch();
		}
		RenderCommand::ToggleFaceCulling(true);
	}

	void SceneRenderer::Prepare()
	{
		RenderCommand::SetClearColor({ 0.2f, 0.2f, 0.2f, 1.0f });
		RenderCommand::Clear();

		m_LightBuffer.Reset();
		m_LightUniformBuffer->SetData(&m_LightBuffer);

		m_Batcher.StartBatch();

		Renderer::ResetStats();
	}

	void SceneRenderer::FlushDebugDraws()
	{
		DDRenderInterface::Flush(EngineUtils::GetTimeInSeconds() * 1000.0f);
	}

	void SceneRenderer::OnViewportResize(uint32_t width, uint32_t height)
	{
		m_Ddri->UpdateViewportSize(width, height);
	}

	void SceneRenderer::RenderGrid()
	{
		if (!m_bDrawGrid) return;

		m_GridShader->Bind();
		m_GridUniformBuffer->Bind();
		RenderCommand::DrawInstanced(m_GridBuffer.InstanceCount);
	}

	void SceneRenderer::SetupDirectionalLight(const glm::vec3& rotation, const Ref<DirectionalLight>& directionalLight)
	{
		m_LightBuffer.DirectionalLightBuffer.Color = directionalLight->GetColor();
		m_LightBuffer.DirectionalLightBuffer.Intensity = directionalLight->GetIntensity();
		m_LightBuffer.DirectionalLightBuffer.Direction = directionalLight->CalculateDirection(rotation);
	}

	void SceneRenderer::AddPointLight(const glm::vec3& position, const Ref<PointLight>& pointLight)
	{
		m_LightBuffer.PointLightBuffer[m_LightBuffer.NumPointLights].Color = pointLight->GetColor();
		m_LightBuffer.PointLightBuffer[m_LightBuffer.NumPointLights].Intensity = pointLight->GetIntensity();
		m_LightBuffer.PointLightBuffer[m_LightBuffer.NumPointLights].Position = position;
		m_LightBuffer.PointLightBuffer[m_LightBuffer.NumPointLights].Radius = pointLight->GetRange();
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
		++m_LightBuffer.NumSpotLights;
	}

	void SceneRenderer::UploadLightData()
	{
		m_LightUniformBuffer->SetData(&m_LightBuffer);
	}

	void SceneRenderer::DrawMesh(const glm::mat4& transform, const Ref<Mesh>& mesh, int32_t entityID)
	{
		if (!mesh) return;

		mesh->Submit(transform, entityID);
		Renderer::GetStats().MeshVertexCount += mesh->GetVertexCount();
	}

	void SceneRenderer::DrawQuad(const glm::mat4& transform, const glm::vec4& color, int32_t entityID)
	{
		m_Batcher.DrawQuad(transform, color, entityID);
	}

	void SceneRenderer::DrawQuad(const glm::mat4& transform, const Ref<Texture2D>& texture, const glm::vec2& tilingFactor, const glm::vec2& uvOffset, const glm::vec4& tintColor, int32_t entityID)
	{
		m_Batcher.DrawQuad(transform, texture, tilingFactor, uvOffset, tintColor, entityID);
	}

	void SceneRenderer::DrawBillboard(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec2& tilingFactor, const glm::vec2& uvOffset, const glm::vec4& tintColor, int32_t entityID)
	{
		glm::mat4 lookAtMatrix = glm::lookAt(position, m_CameraBuffer.Position, { 0.0f, 1.0f, 0.0f });
		glm::mat4 transform = glm::inverse(lookAtMatrix) *
			glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
		glm::vec4 color = tintColor;
		color.a = 1.0f;
		m_Batcher.DrawQuad(transform, texture, tilingFactor, uvOffset, color, entityID);
	}

}

#include "ZEpch.h"
#include "Engine/Renderer/Renderer.h"

#include "Engine/Renderer/Renderer2D.h"
#include "Engine/Debug/BenchmarkTimer.h"

namespace ZeoEngine {

	RendererData Renderer::s_Data;

	void Renderer::Init()
	{
		ZE_PROFILE_FUNCTION();

		RenderCommand::Init();

		if (RendererAPI::Is2D())
		{
			Renderer2D::Init();
		}
		else
		{
			s_Data.DefaultShader = ShaderAssetLibrary::GetDefaultShaderAsset()->GetShader();
			s_Data.CameraUniformBuffer = UniformBuffer::Create(sizeof(RendererData::CameraData), 0);
			s_Data.LightUniformBuffer = UniformBuffer::Create(sizeof(RendererData::LightData), 2);
		}
	}

	void Renderer::Shutdown()
	{
		if (RendererAPI::Is2D())
		{
			Renderer2D::Shutdown();
		}
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		RenderCommand::SetViewport(0, 0, width, height);
	}

	void Renderer::BeginScene(const Camera& camera, const glm::mat4& transform)
	{
		s_Data.CameraBuffer.ViewProjection = camera.GetProjection() * glm::inverse(transform);
		s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(RendererData::CameraData));
		
		Prepare();
	}

	void Renderer::BeginScene(const EditorCamera& camera)
	{
		s_Data.CameraBuffer.ViewProjection = camera.GetViewProjection();
		s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(RendererData::CameraData));

		Prepare();
	}

	void Renderer::EndScene()
	{
		Submit();
	}

	void Renderer::Prepare()
	{
		s_Data.RenderQueue.clear();
		s_Data.LightBuffer.Reset();
		s_Data.LightUniformBuffer->SetData(&s_Data.LightBuffer, sizeof(RendererData::LightData));
	}

	void Renderer::Submit()
	{
		for (auto& renderData : s_Data.RenderQueue)
		{
			renderData.Bind();
			for (uint32_t i = 0; i < renderData.MeshCount; ++i)
			{
				const auto& meshEntry = renderData.MeshEntries[i];
				const auto& material = renderData.Materials[meshEntry.MaterialIndex];
				if (material && material->GetShader())
				{
					material->GetShader()->Bind();
				}
				else
				{
					s_Data.DefaultShader->Bind();
				}
				
				RenderCommand::DrawIndexed(renderData.VAO, meshEntry.VertexBufferPtr, meshEntry.IndexCount, meshEntry.IndexBufferPtr);
			}
		}
	}

	void Renderer::SetupDirectionalLight(const glm::vec3& position, const glm::vec3& rotation, const Ref<DirectionalLight>& directionalLight)
	{
		s_Data.LightBuffer.DirectionalLightPosition = position;
		s_Data.LightBuffer.DirectionalLightDirection = directionalLight->CalculateDirection(rotation);
		s_Data.LightBuffer.DirectionalLightColor = directionalLight->GetColor();
		s_Data.LightBuffer.DirectionalLightIntensity = directionalLight->GetIntensity();
		s_Data.LightUniformBuffer->SetData(&s_Data.LightBuffer, sizeof(RendererData::LightData));
	}

	void Renderer::DrawMesh(const glm::mat4& transform, const Ref<Mesh>& mesh, int32_t entityID)
	{
		if (!mesh) return;

		s_Data.RenderQueue.emplace_back(mesh->GetVAO(), mesh->GetMeshEntries(), mesh->GetMeshCount(), mesh->GetMaterials().data(), mesh->GetMaterialCount(), transform, entityID);
	}

	Statistics& Renderer::GetStats()
	{
		return s_Data.Stats;
	}

	void Renderer::ResetStats()
	{
		s_Data.Stats.Reset();
	}

}

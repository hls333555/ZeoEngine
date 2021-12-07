#include "ZEpch.h"
#include "Engine/Renderer/Renderer.h"

#include "Engine/Renderer/Renderer2D.h"
#include "Engine/Renderer/DebugDrawRenderInterface.h"
#include "Engine/Profile/BenchmarkTimer.h"
#include "Engine/Utils/EngineUtils.h"

namespace ZeoEngine {

	RendererData Renderer::s_Data;
	Scope<DDRenderInterface> Renderer::s_Ddri = DDRenderInterface::Create();

	void Renderer::Init()
	{
		ZE_PROFILE_FUNCTION();

		RenderCommand::Init();

		// Init debug draw render interface
		DDRenderInterface::Init(s_Ddri);

		if (RendererAPI::Is2D())
		{
			Renderer2D::Init();
		}
		else
		{
			// Quads
			{
				s_Data.PrimitiveBuffer.QuadVAO = VertexArray::Create();

				s_Data.PrimitiveBuffer.QuadVBO = VertexBuffer::Create(s_Data.PrimitiveBuffer.MaxVertices * sizeof(QuadVertex));
				BufferLayout quadLayout = {
					{ ShaderDataType::Float3, "a_Position"     },
					{ ShaderDataType::Float4, "a_Color"        },
					{ ShaderDataType::Float2, "a_TexCoord"     },
					{ ShaderDataType::Float2, "a_TilingFactor" },
					{ ShaderDataType::Float2, "a_UvOffset"     },
					{ ShaderDataType::Float,  "a_TexIndex"     },
					{ ShaderDataType::Int,    "a_EntityID"     },
				};
				s_Data.PrimitiveBuffer.QuadVBO->SetLayout(quadLayout);
				s_Data.PrimitiveBuffer.QuadVAO->AddVertexBuffer(s_Data.PrimitiveBuffer.QuadVBO);

				s_Data.PrimitiveBuffer.QuadVertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
				s_Data.PrimitiveBuffer.QuadVertexPositions[1] = {  0.5f, -0.5f, 0.0f, 1.0f };
				s_Data.PrimitiveBuffer.QuadVertexPositions[2] = {  0.5f,  0.5f, 0.0f, 1.0f };
				s_Data.PrimitiveBuffer.QuadVertexPositions[3] = { -0.5f,  0.5f, 0.0f, 1.0f };
				s_Data.PrimitiveBuffer.QuadVertexBufferBase = new QuadVertex[s_Data.PrimitiveBuffer.MaxVertices];

				uint32_t* quadIndices = new uint32_t[s_Data.PrimitiveBuffer.MaxIndices];
				uint32_t offset = 0;
				for (uint32_t i = 0; i < s_Data.PrimitiveBuffer.MaxIndices; i += 6)
				{
					quadIndices[i + 0] = offset + 0;
					quadIndices[i + 1] = offset + 1;
					quadIndices[i + 2] = offset + 2;

					quadIndices[i + 3] = offset + 2;
					quadIndices[i + 4] = offset + 3;
					quadIndices[i + 5] = offset + 0;

					offset += 4;
				}
				Ref<IndexBuffer> quadIBO = IndexBuffer::Create(quadIndices, s_Data.PrimitiveBuffer.MaxIndices);
				s_Data.PrimitiveBuffer.QuadVAO->SetIndexBuffer(quadIBO);
				delete[] quadIndices;

				s_Data.PrimitiveBuffer.QuadShader = Shader::Create("assets/editor/shaders/Quad.glsl");
			}

			// Generate a 1x1 white texture to be used by flat color
			s_Data.PrimitiveBuffer.WhiteTexture = Texture2D::Create(1, 1);
			uint32_t whiteTextureData = 0xffffffff;
			s_Data.PrimitiveBuffer.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));
			s_Data.PrimitiveBuffer.TextureSlots[0] = s_Data.PrimitiveBuffer.WhiteTexture;

			s_Data.DefaultMaterial = MaterialAssetLibrary::GetDefaultMaterialAsset()->GetMaterial();
			s_Data.CameraUniformBuffer = UniformBuffer::Create(sizeof(RendererData::CameraData), 0);

			s_Data.GridShader = Shader::Create("assets/editor/shaders/Grid.glsl");
			s_Data.GridUniformBuffer = UniformBuffer::Create(sizeof(RendererData::GridData), 1);
			s_Data.GridUniformBuffer->SetData(&s_Data.GridBuffer);

			s_Data.LightUniformBuffer = UniformBuffer::Create(sizeof(RendererData::LightData), 2);
		}
	}

	void Renderer::Shutdown()
	{
		DDRenderInterface::Shutdown();

		if (RendererAPI::Is2D())
		{
			Renderer2D::Shutdown();
		}
		else
		{
			delete[] s_Data.PrimitiveBuffer.QuadVertexBufferBase;
		}
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		RenderCommand::SetViewport(0, 0, width, height);
	}

	void Renderer::OnViewportResize(uint32_t width, uint32_t height)
	{
		s_Ddri->UpdateViewportSize(width, height);
	}

	void Renderer::BeginScene(const Camera& camera, const glm::mat4& transform)
	{
		s_Data.CameraBuffer.View = glm::inverse(transform);
		s_Data.CameraBuffer.Projection = camera.GetProjection();
		s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer);
		
		Prepare();
	}

	void Renderer::BeginScene(const EditorCamera& camera, bool bDrawGrid)
	{
		s_Data.CameraBuffer.View = camera.GetViewMatrix();
		s_Data.CameraBuffer.Projection = camera.GetProjection();
		s_Data.CameraBuffer.Position = camera.GetPosition();
		s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer);

		s_Data.bDrawGrid = bDrawGrid;

		Prepare();
	}

	void Renderer::EndScene()
	{
		Renderer::UploadLightData();
		{
			Submit();
		}
		RenderCommand::ToggleFaceCulling(false);
		RenderCommand::ToggleDepthWriting(false);
		{
			// Translucent grid
			RenderGrid();
		}
		RenderCommand::ToggleDepthWriting(true);
		{
			// Quads are drawn at last
			Renderer::FlushBatch();
		}
		RenderCommand::ToggleFaceCulling(true);
	}

	void Renderer::FlushDebugDraws()
	{
		DDRenderInterface::Flush(EngineUtils::GetTimeInSeconds() * 1000.0f);
	}

	void Renderer::Prepare()
	{
		s_Data.RenderQueue.clear();
		s_Data.LightBuffer.Reset();
		s_Data.LightUniformBuffer->SetData(&s_Data.LightBuffer);

		StartBatch();
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
				if (material)
				{
					material->Bind();
				}
				else
				{
					s_Data.DefaultMaterial->Bind();
				}
				RenderCommand::DrawIndexed(renderData.VAO, meshEntry.VertexBufferPtr, meshEntry.IndexCount, meshEntry.IndexBufferPtr);
				++s_Data.Stats.DrawCalls;
			}
		}
	}

	void Renderer::StartBatch()
	{
		s_Data.PrimitiveBuffer.QuadIndexCount = 0;
		s_Data.PrimitiveBuffer.QuadVertexBufferPtr = s_Data.PrimitiveBuffer.QuadVertexBufferBase;

		s_Data.PrimitiveBuffer.TextureSlotIndex = 1;

		ResetStats();
	}

	void Renderer::NextBatch()
	{
		FlushBatch();
		StartBatch();
	}

	void Renderer::FlushBatch()
	{
		if (s_Data.PrimitiveBuffer.QuadIndexCount)
		{
			auto dataSize = reinterpret_cast<uint8_t*>(s_Data.PrimitiveBuffer.QuadVertexBufferPtr) - reinterpret_cast<uint8_t*>(s_Data.PrimitiveBuffer.QuadVertexBufferBase);
			s_Data.PrimitiveBuffer.QuadVBO->SetData(s_Data.PrimitiveBuffer.QuadVertexBufferBase, static_cast<uint32_t>(dataSize));

			for (uint32_t i = 0; i < s_Data.PrimitiveBuffer.TextureSlotIndex; i++)
			{
				s_Data.PrimitiveBuffer.TextureSlots[i]->Bind(i);
			}
			s_Data.PrimitiveBuffer.QuadShader->Bind();
			RenderCommand::DrawIndexed(s_Data.PrimitiveBuffer.QuadVAO, s_Data.PrimitiveBuffer.QuadIndexCount);
			++s_Data.Stats.DrawCalls;
		}
	}

	void Renderer::RenderGrid()
	{
		if (!s_Data.bDrawGrid) return;

		s_Data.GridShader->Bind();
		s_Data.GridUniformBuffer->Bind();
		RenderCommand::DrawInstanced(s_Data.GridBuffer.InstanceCount);
		++s_Data.Stats.DrawCalls;
	}

	void Renderer::SetupDirectionalLight(const glm::vec3& rotation, const Ref<DirectionalLight>& directionalLight)
	{
		s_Data.LightBuffer.DirectionalLightBuffer.Color = directionalLight->GetColor();
		s_Data.LightBuffer.DirectionalLightBuffer.Intensity = directionalLight->GetIntensity();
		s_Data.LightBuffer.DirectionalLightBuffer.Direction = directionalLight->CalculateDirection(rotation);
	}

	void Renderer::AddPointLight(const glm::vec3& position, const Ref<PointLight>& pointLight)
	{
		s_Data.LightBuffer.PointLightBuffer[s_Data.LightBuffer.NumPointLights].Color = pointLight->GetColor();
		s_Data.LightBuffer.PointLightBuffer[s_Data.LightBuffer.NumPointLights].Intensity = pointLight->GetIntensity();
		s_Data.LightBuffer.PointLightBuffer[s_Data.LightBuffer.NumPointLights].Position = position;
		s_Data.LightBuffer.PointLightBuffer[s_Data.LightBuffer.NumPointLights].Radius = pointLight->GetRadius();
		++s_Data.LightBuffer.NumPointLights;
	}

	void Renderer::UploadLightData()
	{
		s_Data.LightUniformBuffer->SetData(&s_Data.LightBuffer);
	}

	void Renderer::DrawMesh(const glm::mat4& transform, const Ref<Mesh>& mesh, int32_t entityID)
	{
		if (!mesh) return;

		s_Data.RenderQueue.emplace_back(mesh->GetVAO(), mesh->GetMeshEntries(), mesh->GetMeshCount(), mesh->GetMaterials().data(), mesh->GetMaterialCount(), transform, entityID);
		s_Data.Stats.MeshVertexCount += mesh->GetVertexCount();
	}

	void Renderer::DrawQuad(const glm::mat4& transform, const glm::vec4& color, int32_t entityID)
	{
		ZE_PROFILE_FUNCTION();

		if (s_Data.PrimitiveBuffer.QuadIndexCount >= s_Data.PrimitiveBuffer.MaxIndices)
		{
			NextBatch();
		}

		constexpr size_t quadVertexCount = 4;
		constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
		// White texture
		constexpr float textureIndex = 0.0f;
		constexpr glm::vec2 tilingFactor = { 1.0f, 1.0f };
		constexpr glm::vec2 uvOffset = { 0.0f, 0.0f };

		for (size_t i = 0; i < quadVertexCount; ++i)
		{
			s_Data.PrimitiveBuffer.QuadVertexBufferPtr->Position = transform * s_Data.PrimitiveBuffer.QuadVertexPositions[i];
			s_Data.PrimitiveBuffer.QuadVertexBufferPtr->Color = color;
			s_Data.PrimitiveBuffer.QuadVertexBufferPtr->TexCoord = textureCoords[i];
			s_Data.PrimitiveBuffer.QuadVertexBufferPtr->TexIndex = textureIndex;
			s_Data.PrimitiveBuffer.QuadVertexBufferPtr->TilingFactor = tilingFactor;
			s_Data.PrimitiveBuffer.QuadVertexBufferPtr->UvOffset = uvOffset;
			s_Data.PrimitiveBuffer.QuadVertexBufferPtr->EntityID = entityID;
			++s_Data.PrimitiveBuffer.QuadVertexBufferPtr;
		}

		s_Data.PrimitiveBuffer.QuadIndexCount += 6;

		++s_Data.Stats.QuadCount;
	}

	void Renderer::DrawQuad(const glm::mat4& transform, const Ref<Texture2D>& texture, const glm::vec2& tilingFactor, const glm::vec2& uvOffset, const glm::vec4& tintColor, int32_t entityID)
	{
		ZE_PROFILE_FUNCTION();

		if (s_Data.PrimitiveBuffer.QuadIndexCount >= s_Data.PrimitiveBuffer.MaxIndices)
		{
			NextBatch();
		}

		float textureIndex = 0.0f;
		for (uint32_t i = 1; i < s_Data.PrimitiveBuffer.TextureSlotIndex; ++i)
		{
			if (*s_Data.PrimitiveBuffer.TextureSlots[i] == *texture)
			{
				textureIndex = static_cast<float>(i);
				break;
			}
		}
		if (textureIndex == 0.0f)
		{
			if (s_Data.PrimitiveBuffer.TextureSlotIndex >= Renderer2DData::MaxTextureSlots)
			{
				NextBatch();
			}

			textureIndex = static_cast<float>(s_Data.PrimitiveBuffer.TextureSlotIndex);
			s_Data.PrimitiveBuffer.TextureSlots[s_Data.PrimitiveBuffer.TextureSlotIndex++] = texture;
		}

		constexpr size_t quadVertexCount = 4;
		constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };

		for (size_t i = 0; i < quadVertexCount; i++)
		{
			s_Data.PrimitiveBuffer.QuadVertexBufferPtr->Position = transform * s_Data.PrimitiveBuffer.QuadVertexPositions[i];
			s_Data.PrimitiveBuffer.QuadVertexBufferPtr->Color = tintColor;
			s_Data.PrimitiveBuffer.QuadVertexBufferPtr->TexCoord = textureCoords[i];
			s_Data.PrimitiveBuffer.QuadVertexBufferPtr->TexIndex = textureIndex;
			s_Data.PrimitiveBuffer.QuadVertexBufferPtr->TilingFactor = tilingFactor;
			s_Data.PrimitiveBuffer.QuadVertexBufferPtr->UvOffset = uvOffset;
			s_Data.PrimitiveBuffer.QuadVertexBufferPtr->EntityID = entityID;
			++s_Data.PrimitiveBuffer.QuadVertexBufferPtr;
		}

		s_Data.PrimitiveBuffer.QuadIndexCount += 6;

		++s_Data.Stats.QuadCount;
	}

	void Renderer::DrawBillboard(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec2& tilingFactor, const glm::vec2& uvOffset, const glm::vec4& tintColor, int32_t entityID)
	{
		glm::mat4 lookAtMatrix = glm::lookAt(position, s_Data.CameraBuffer.Position, { 0.0f, 1.0f, 0.0f });
		glm::mat4 transform = glm::inverse(lookAtMatrix) *
			glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
		glm::vec4 color = tintColor;
		color.a = 1.0f;
		DrawQuad(transform, texture, tilingFactor, uvOffset, color, entityID);
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

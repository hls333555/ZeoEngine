#pragma once

#include "Engine/Renderer/EditorCamera.h"
#include "Engine/Renderer/RenderCommand.h"
#include "Engine/Renderer/Shader.h"
#include "Engine/Renderer/Mesh.h"
#include "Engine/Renderer/Material.h"

namespace ZeoEngine {

	struct MeshRendererComponent;
	struct QuadVertex;

	struct RenderData
	{
		Ref<VertexArray> VAO;
		const MeshEntry* MeshEntries;
		uint32_t MeshCount = 0;
		const AssetHandle<MaterialAsset>* Materials;
		uint32_t MaterialCount = 0;
		struct ModelData
		{
			glm::mat4 Transform;
			glm::mat4 NormalMatrix;

			// Editor-only
			int32_t EntityID;
		};
		ModelData ModelBuffer;
		Ref<UniformBuffer> ModelUniformBuffer;

		RenderData(const Ref<VertexArray>& vao, const MeshEntry* meshEntries, uint32_t meshCount, const AssetHandle<MaterialAsset>* materials, uint32_t materialCount, const glm::mat4& transform, int32_t entityID)
			: VAO(vao), MeshEntries(meshEntries), MeshCount(meshCount), Materials(materials), MaterialCount(materialCount)
		{
			ModelUniformBuffer = UniformBuffer::Create(sizeof(ModelData), 1);
			ModelBuffer.Transform = transform;
			ModelBuffer.NormalMatrix = glm::transpose(glm::inverse(transform));
			ModelBuffer.EntityID = entityID;
			ModelUniformBuffer->SetData(&ModelBuffer);
		}

		void Bind()
		{
			ModelUniformBuffer->Bind();
		}

	};

	struct Statistics
	{
		uint32_t DrawCalls = 0;
		uint32_t QuadCount = 0;

		Entity HoveredEntity;

		uint32_t GetTotalVertexCount() const { return QuadCount * 4; }
		uint32_t GetTotalIndexCount() const { return QuadCount * 6; }

		void Reset()
		{
			DrawCalls = 0;
			QuadCount = 0;
			HoveredEntity = {};
		}
	};

	struct QuadData
	{
		const uint32_t MaxQuads = 10000;
		const uint32_t MaxVertices = MaxQuads * 4;
		const uint32_t MaxIndices = MaxQuads * 6;
		static const uint32_t MaxTextureSlots = 32;

		Ref<VertexArray> QuadVAO;
		Ref<VertexBuffer> QuadVBO;
		Ref<Shader> QuadShader;

		uint32_t QuadIndexCount = 0;
		QuadVertex* QuadVertexBufferBase = nullptr;
		QuadVertex* QuadVertexBufferPtr = nullptr;

		glm::vec4 QuadVertexPositions[4];

		std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
		Ref<Texture2D> WhiteTexture;
		uint32_t TextureSlotIndex = 1; // 0 = white texture
	};

	struct RendererData
	{
		QuadData QuadBuffer;

		Ref<Material> DefaultMaterial;
		std::vector<RenderData> RenderQueue;

		struct CameraData
		{
			glm::mat4 View;
			glm::mat4 Projection;
			glm::vec3 Position;
		};
		CameraData CameraBuffer;
		Ref<UniformBuffer> CameraUniformBuffer;

		// Data alignment is required for std140 layout!
		struct LightData
		{
			glm::vec4 DirectionalLightColor;
			glm::vec3 DirectionalLightPosition;
			float DirectionalLightIntensity;
			glm::vec3 DirectionalLightDirection;

			void Reset()
			{
				DirectionalLightColor = glm::vec4{ 0.0f };
				DirectionalLightPosition = glm::vec3{ 0.0f };
				DirectionalLightIntensity = 0.0f;
				DirectionalLightDirection = glm::vec3{ 0.0f };
			}
		};
		LightData LightBuffer;
		Ref<UniformBuffer> LightUniformBuffer;

		Statistics Stats;
	};

	class Renderer
	{
	public:
		static void Init();
		static void Shutdown();

		static void OnWindowResize(uint32_t width, uint32_t height);

		static void BeginScene(const Camera& camera, const glm::mat4& transform);
		static void BeginScene(const EditorCamera& camera);
		static void EndScene();

		static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }

		static void SetupDirectionalLight(const glm::vec3& position, const glm::vec3& rotation, const Ref<DirectionalLight>& directionalLight);

		static void DrawMesh(const glm::mat4& transform, const Ref<Mesh>& mesh, int32_t entityID = -1);

		static void DrawQuad(const glm::mat4& transform, const glm::vec4& color, int32_t entityID = -1);
		static void DrawQuad(const glm::mat4& transform, const AssetHandle<Texture2DAsset>& texture, const glm::vec2& tilingFactor = { 1.0f, 1.0f }, const glm::vec2& uvOffset = { 0.0f, 0.0f }, const glm::vec4& tintColor = glm::vec4(1.0f), int32_t entityID = -1);

		// TODO:
		static Statistics& GetStats();
		static void ResetStats();

	private:
		static void Prepare();
		static void Submit();

		static void StartBatch();
		static void NextBatch();
		static void FlushBatch();

	private:
		static RendererData s_Data;
	};

}

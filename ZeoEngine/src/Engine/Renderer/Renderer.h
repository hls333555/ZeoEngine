#pragma once

#include "Engine/Renderer/EditorCamera.h"
#include "Engine/Renderer/RenderCommand.h"
#include "Engine/Renderer/Shader.h"
#include "Engine/Renderer/Mesh.h"
#include "Engine/Renderer/Material.h"

namespace ZeoEngine {

	struct MeshRendererComponent;
	class DDRenderInterface;

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
		uint32_t LineVertexCount = 0;
		uint32_t MeshVertexCount = 0;

		Entity HoveredEntity;

		uint32_t GetTotalVertexCount() const { return QuadCount * 4 + LineVertexCount + MeshVertexCount; }

		void Reset()
		{
			DrawCalls = 0;
			QuadCount = 0;
			LineVertexCount = 0;
			MeshVertexCount = 0;
			HoveredEntity = {};
		}
	};

	struct QuadVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;
		glm::vec2 TexCoord;
		glm::vec2 TilingFactor;
		glm::vec2 UvOffset;
		float TexIndex;

		// Editor-only
		int32_t EntityID;
	};

	struct PrimitiveData
	{
		const uint32_t MaxQuads = 10000;
		const uint32_t MaxVertices = MaxQuads * 4;
		const uint32_t MaxIndices = MaxQuads * 6;
		static const uint32_t MaxTextureSlots = 32;

		Ref<VertexArray> QuadVAO;
		Ref<VertexBuffer> QuadVBO;
		glm::vec4 QuadVertexPositions[4];
		Ref<Shader> QuadShader;
		uint32_t QuadIndexCount = 0;
		QuadVertex* QuadVertexBufferBase = nullptr;
		QuadVertex* QuadVertexBufferPtr = nullptr;

		std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
		Ref<Texture2D> WhiteTexture;
		uint32_t TextureSlotIndex = 1; // 0 = white texture
	};

	// Data alignment is required for std140 layout!
	struct RendererData
	{
		PrimitiveData PrimitiveBuffer;

		Ref<Material> DefaultMaterial;
		std::vector<RenderData> RenderQueue;

		struct CameraData
		{
			glm::mat4 View;
			glm::mat4 Projection;
			glm::vec3 Position;

			glm::mat4 GetViewProjection() const { return Projection * View; }
		};
		CameraData CameraBuffer;
		Ref<UniformBuffer> CameraUniformBuffer;

		struct GridData
		{
			glm::mat4 Transform = glm::mat4(1.0f);
			glm::vec4 ThinLinesColor{ 0.2f, 0.2f, 0.2f, 0.3f };
			glm::vec4 ThickLinesColor{ 0.5f, 0.5f, 0.5f, 0.3f };
			glm::vec4 OriginAxisXColor{ 1.0f, 0.0f, 0.0f, 0.3f };
			glm::vec4 OriginAxisZColor{ 0.0f, 0.0f, 1.0f, 0.3f };
			float Extent = 101.0f;
			float CellSize = 0.025f;
			int32_t InstanceCount = 10;
		};
		GridData GridBuffer;
		Ref<UniformBuffer> GridUniformBuffer;
		Ref<Shader> GridShader;
		bool bDrawGrid = false;

		static const int32_t MAX_POINT_LIGHTS = 32;
		struct LightDataBase
		{
			glm::vec4 Color;
			float Intensity, Padding, Padding1, Padding2; // NOTE: The paddings are only needed in code

			void Reset()
			{
				Color = glm::vec4{ 0.0f };
				Intensity = 0.0f;
			}
		};

		struct DirectionalLightData : public LightDataBase
		{
			glm::vec3 Direction; float Padding;

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

		struct LightData
		{
			DirectionalLightData DirectionalLightBuffer;
			PointLightData PointLightBuffer[MAX_POINT_LIGHTS];
			int32_t NumPointLights = 0;

			void Reset()
			{
				DirectionalLightBuffer.Reset();
				NumPointLights = 0;
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
		static void OnViewportResize(uint32_t width, uint32_t height);

		static void BeginScene(const Camera& camera, const glm::mat4& transform);
		static void BeginScene(const EditorCamera& camera, bool bDrawGrid = false);
		static void EndScene();

		static void FlushDebugDraws();

		static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }

		static void RenderGrid();

		static void SetupDirectionalLight(const glm::vec3& rotation, const Ref<DirectionalLight>& directionalLight);
		static void AddPointLight(const glm::vec3& position, const Ref<PointLight>& pointLight);

		static void DrawMesh(const glm::mat4& transform, const Ref<Mesh>& mesh, int32_t entityID = -1);

		static void DrawQuad(const glm::mat4& transform, const glm::vec4& color, int32_t entityID = -1);
		static void DrawQuad(const glm::mat4& transform, const Ref<Texture2D>& texture, const glm::vec2& tilingFactor = { 1.0f, 1.0f }, const glm::vec2& uvOffset = { 0.0f, 0.0f }, const glm::vec4& tintColor = glm::vec4(1.0f), int32_t entityID = -1);
		static void DrawBillboard(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec2& tilingFactor = { 1.0f, 1.0f }, const glm::vec2& uvOffset = { 0.0f, 0.0f }, const glm::vec4& tintColor = glm::vec4(1.0f), int32_t entityID = -1);

		static Statistics& GetStats();
		static void ResetStats();

		static glm::mat4 GetViewProjectionMatrix() { return s_Data.CameraBuffer.GetViewProjection(); }

	private:
		static void Prepare();
		static void Submit();
		static void UploadLightData();

		static void StartBatch();
		static void NextBatch();
		static void FlushBatch();

	private:
		static RendererData s_Data;
		static Scope<DDRenderInterface> s_Ddri;
	};

}

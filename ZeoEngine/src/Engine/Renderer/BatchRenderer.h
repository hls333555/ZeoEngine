#pragma once

#include <glm/glm.hpp>

#include "Engine/Core/EngineTypes.h"

namespace ZeoEngine {

	class VertexArray;
	class VertexBuffer;
	class Shader;
	class Texture2D;

	class BatchRenderer
	{
	public:
		~BatchRenderer();

		void Init();
		void StartBatch();
		void FlushBatch() const;

		void DrawQuad(const glm::mat4& transform, const glm::vec4& color, int32_t entityID = -1);
		void DrawQuad(const glm::mat4& transform, const AssetHandle<Texture2D>& texture, const glm::vec2& tilingFactor = { 1.0f, 1.0f }, const glm::vec2& uvOffset = { 0.0f, 0.0f }, const glm::vec4& tintColor = glm::vec4(1.0f), int32_t entityID = -1);

	private:
		void NextBatch();

	private:
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
			static constexpr uint32_t MaxTextureSlots = 32;

			Ref<VertexArray> QuadVAO;
			Ref<VertexBuffer> QuadVBO;
			glm::vec4 QuadVertexPositions[4];
			Ref<Shader> QuadShader;
			uint32_t QuadIndexCount = 0;
			QuadVertex* QuadVertexBufferBase = nullptr;
			QuadVertex* QuadVertexBufferPtr = nullptr;

			std::array<AssetHandle<Texture2D>, MaxTextureSlots> TextureSlots;
			uint32_t TextureSlotIndex = 1; // 0 = white texture
		};
		PrimitiveData m_PrimitiveBuffer;
	};

}

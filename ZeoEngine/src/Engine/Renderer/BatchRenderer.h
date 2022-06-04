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

		void DrawQuad(const Mat4& transform, const Vec4& color, I32 entityID = -1);
		void DrawQuad(const Mat4& transform, const AssetHandle<Texture2D>& texture, const Vec2& tilingFactor = { 1.0f, 1.0f }, const Vec2& uvOffset = { 0.0f, 0.0f }, const Vec4& tintColor = Vec4(1.0f), I32 entityID = -1);

	private:
		void NextBatch();

	private:
		struct QuadVertex
		{
			Vec3 Position;
			Vec4 Color;
			Vec2 TexCoord;
			Vec2 TilingFactor;
			Vec2 UvOffset;
			float TexIndex;

			// Editor-only
			I32 EntityID;
		};

		struct PrimitiveData
		{
			const U32 MaxQuads = 10000;
			const U32 MaxVertices = MaxQuads * 4;
			const U32 MaxIndices = MaxQuads * 6;
			static constexpr U32 MaxTextureSlots = 32;

			Ref<VertexArray> QuadVAO;
			Ref<VertexBuffer> QuadVBO;
			Vec4 QuadVertexPositions[4];
			Ref<Shader> QuadShader;
			U32 QuadIndexCount = 0;
			QuadVertex* QuadVertexBufferBase = nullptr;
			QuadVertex* QuadVertexBufferPtr = nullptr;

			std::array<AssetHandle<Texture2D>, MaxTextureSlots> TextureSlots;
			U32 TextureSlotIndex = 1; // 0 = white texture
		};
		PrimitiveData m_PrimitiveBuffer;
	};

}

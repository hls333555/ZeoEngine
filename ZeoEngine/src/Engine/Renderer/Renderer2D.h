#pragma once

#include "Engine/Renderer/OrthographicCamera.h"
#include "Engine/Renderer/Texture.h"
#include "Engine/Renderer/SubTexture2D.h"
#include "Engine/Renderer/VertexArray.h"
#include "Engine/Renderer/Shader.h"

namespace ZeoEngine {

	struct QuadVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;
		glm::vec2 TexCoord;
		float TexIndex;
		glm::vec2 TilingFactor;
		glm::vec2 UVOffset;
	};

	struct Statistics
	{
		uint32_t DrawCalls = 0;
		uint32_t QuadCount = 0;

		uint32_t GetTotalVertexCount() { return QuadCount * 4; }
		uint32_t GetTotalIndexCount() { return QuadCount * 6; }
	};

	struct Renderer2DData
	{
		const uint32_t MaxQuads = 10000;
		const uint32_t MaxVertices = MaxQuads * 4;
		const uint32_t MaxIndices = MaxQuads * 6;
		static const uint32_t MaxTextureSlots = 32;

		Ref<VertexArray> QuadVAO;
		Ref<VertexBuffer> QuadVBO;
		Ref<FrameBuffer> FBOs[2];

		uint32_t QuadIndexCount = 0;
		QuadVertex* QuadVertexBufferBase = nullptr;
		QuadVertex* QuadVertexBufferPtr = nullptr;
		glm::vec4 QuadVertexPositions[4];

		Ref<Shader> TextureShader;
		Ref<Texture2D> WhiteTexture;
		std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
		uint32_t TextureSlotIndex = 1; // 0 = white texture

		Statistics Stats;
	};

	class Renderer2D
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginRenderingToTexture(uint32_t frameBufferIndex);
		static void EndRenderingToTexture(uint32_t frameBufferIndex);
		static void BeginScene(const OrthographicCamera& camera);
		static void EndScene();
		static void Flush();
	private:
		static void FlushAndReset();

	public:
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec2& tilingFactor = { 1.0f, 1.0f }, const glm::vec2& uvOffset = { 0.0f, 0.0f }, const glm::vec4& tintColor = glm::vec4(1.0f));
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec2& tilingFactor = { 1.0f, 1.0f }, const glm::vec2& uvOffset = { 0.0f, 0.0f }, const glm::vec4& tintColor = glm::vec4(1.0f));
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<SubTexture2D>& subTexture, const glm::vec2& tilingFactor = { 1.0f, 1.0f }, const glm::vec2& uvOffset = { 0.0f, 0.0f }, const glm::vec4& tintColor = glm::vec4(1.0f));
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<SubTexture2D>& subTexture, const glm::vec2& tilingFactor = { 1.0f, 1.0f }, const glm::vec2& uvOffset = { 0.0f, 0.0f }, const glm::vec4& tintColor = glm::vec4(1.0f));

		/** Rotation should be in radians. */
		static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& color);
		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& color);
		static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, const glm::vec2& tilingFactor = { 1.0f, 1.0f }, const glm::vec2& uvOffset = { 0.0f, 0.0f }, const glm::vec4& tintColor = glm::vec4(1.0f));
		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, const glm::vec2& tilingFactor = { 1.0f, 1.0f }, const glm::vec2& uvOffset = { 0.0f, 0.0f }, const glm::vec4& tintColor = glm::vec4(1.0f));
		static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<SubTexture2D>& subTexture, const glm::vec2& tilingFactor = { 1.0f, 1.0f }, const glm::vec2& uvOffset = { 0.0f, 0.0f }, const glm::vec4& tintColor = glm::vec4(1.0f));
		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<SubTexture2D>& subTexture, const glm::vec2& tilingFactor = { 1.0f, 1.0f }, const glm::vec2& uvOffset = { 0.0f, 0.0f }, const glm::vec4& tintColor = glm::vec4(1.0f));
	
		static const Renderer2DData& GetRenderer2DData() { return s_Data; }

		static Statistics GetStats();
		static void ResetStats();

	private:
		static Renderer2DData s_Data;
	};

}

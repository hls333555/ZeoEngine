#pragma once

#include "Engine/Renderer/OrthographicCamera.h"
#include "Engine/Renderer/Texture.h"
#include "Engine/Renderer/VertexArray.h"
#include "Engine/Renderer/Shader.h"

namespace ZeoEngine {

	struct Renderer2DStorage
	{
		Ref<VertexArray> QuadVAO;
		Ref<Shader> TextureShader;
		Ref<Texture> WhiteTexture;
		Ref<FrameBuffer> FBOs[2];
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

		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec2& tilingFactor = { 1.0f, 1.0f }, const glm::vec2& uvOffset = { 0.0f, 0.0f }, const glm::vec4& tintColor = glm::vec4(1.0f));
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec2& tilingFactor = { 1.0f, 1.0f }, const glm::vec2& uvOffset = { 0.0f, 0.0f }, const glm::vec4& tintColor = glm::vec4(1.0f));

		/** Rotation should be in radians. */
		static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& color);
		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& color);
		static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, const glm::vec2& tilingFactor = { 1.0f, 1.0f }, const glm::vec2& uvOffset = { 0.0f, 0.0f }, const glm::vec4& tintColor = glm::vec4(1.0f));
		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, const glm::vec2& tilingFactor = { 1.0f, 1.0f }, const glm::vec2& uvOffset = { 0.0f, 0.0f }, const glm::vec4& tintColor = glm::vec4(1.0f));
	
		static Renderer2DStorage* GetStorageData() { return s_Data; }

	private:
		static Renderer2DStorage* s_Data;
	};

}

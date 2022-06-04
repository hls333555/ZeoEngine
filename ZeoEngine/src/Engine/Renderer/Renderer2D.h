#pragma once

#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/EditorCamera.h"
#include "Engine/Renderer/OrthographicCamera.h"
#include "Engine/Renderer/Texture.h"
#include "Engine/Renderer/SubTexture2D.h"
#include "Engine/Renderer/VertexArray.h"
#include "Engine/Renderer/Shader.h"
#include "Engine/GameFramework/Entity.h"
#include "Engine/GameFramework/Components.h"
#include "Engine/Core/EngineTypes.h"

namespace ZeoEngine {

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

	struct LineVertex
	{
		Vec3 Position;
		Vec4 Color;

		// Editor-only
		I32 EntityID;
	};

	struct CircleVertex
	{
		Vec3 WorldPosition;
		Vec3 LocalPosition;
		Vec4 Color;
		float Thickness;
		float Fade;

		// Editor-only
		I32 EntityID;
	};

	struct Renderer2DData
	{
		const U32 MaxQuads = 10000;
		const U32 MaxVertices = MaxQuads * 4;
		const U32 MaxIndices = MaxQuads * 6;
		static const U32 MaxTextureSlots = 32;

		Ref<VertexArray> QuadVAO;
		Ref<VertexBuffer> QuadVBO;
		Ref<Shader> QuadShader;

		Ref<VertexArray> CircleVAO;
		Ref<VertexBuffer> CircleVBO;
		Ref<Shader> CircleShader;

		Ref<VertexArray> LineVAO;
		Ref<VertexBuffer> LineVBO;
		Ref<Shader> LineShader;

		U32 QuadIndexCount = 0;
		QuadVertex* QuadVertexBufferBase = nullptr;
		QuadVertex* QuadVertexBufferPtr = nullptr;

		U32 CircleIndexCount = 0;
		CircleVertex* CircleVertexBufferBase = nullptr;
		CircleVertex* CircleVertexBufferPtr = nullptr;

		U32 LineVertexCount = 0;
		LineVertex* LineVertexBufferBase = nullptr;
		LineVertex* LineVertexBufferPtr = nullptr;
		float LineThickness = 2.0f;

		Vec4 QuadVertexPositions[4];

		std::array<AssetHandle<Texture2D>, MaxTextureSlots> TextureSlots;
		U32 TextureSlotIndex = 1; // 0 = white texture

		struct CameraData
		{
			Mat4 ViewProjection;
		};
		CameraData CameraBuffer;
		Ref<UniformBuffer> CameraUniformBuffer;

		RendererStats Stats;
	};

	class Renderer2D
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(const Camera& camera, const Mat4& transform);
		static void BeginScene(const EditorCamera& camera);
		static void BeginScene(const OrthographicCamera& camera); // TODO: Remove
		static void EndScene();
	private:
		static void Flush();
		static void StartBatch();
		static void NextBatch();

	public:
		// Primitives
		static void DrawQuad(const Vec2& position, const Vec2& size, const Vec4& color);
		static void DrawQuad(const Vec3& position, const Vec2& size, const Vec4& color);
		static void DrawQuad(const Vec2& position, const Vec2& size, const AssetHandle<Texture2D>& texture, const Vec2& tilingFactor = { 1.0f, 1.0f }, const Vec2& uvOffset = { 0.0f, 0.0f }, const Vec4& tintColor = Vec4(1.0f));
		static void DrawQuad(const Vec3& position, const Vec2& size, const AssetHandle<Texture2D>& texture, const Vec2& tilingFactor = { 1.0f, 1.0f }, const Vec2& uvOffset = { 0.0f, 0.0f }, const Vec4& tintColor = Vec4(1.0f));
		static void DrawQuad(const Vec2& position, const Vec2& size, const Ref<SubTexture2D>& subTexture, const Vec2& tilingFactor = { 1.0f, 1.0f }, const Vec2& uvOffset = { 0.0f, 0.0f }, const Vec4& tintColor = Vec4(1.0f));
		static void DrawQuad(const Vec3& position, const Vec2& size, const Ref<SubTexture2D>& subTexture, const Vec2& tilingFactor = { 1.0f, 1.0f }, const Vec2& uvOffset = { 0.0f, 0.0f }, const Vec4& tintColor = Vec4(1.0f));

		static void DrawQuad(const Mat4& transform, const Vec4& color, I32 entityID = -1);
		static void DrawQuad(const Mat4& transform, const AssetHandle<Texture2D>& texture, const Vec2& tilingFactor = { 1.0f, 1.0f }, const Vec2& uvOffset = { 0.0f, 0.0f }, const Vec4& tintColor = Vec4(1.0f), I32 entityID = -1);
		static void DrawQuad(const Mat4& transform, const Ref<SubTexture2D>& subTexture, const Vec2& tilingFactor = { 1.0f, 1.0f }, const Vec2& uvOffset = { 0.0f, 0.0f }, const Vec4& tintColor = Vec4(1.0f), I32 entityID = -1);

		static void DrawCircle(const Mat4& transform, const Vec4& color, float thickness = 1.0f, float fade = 0.005f, I32 entityID = -1);

		static void DrawLine(const Vec3& p0, const Vec3& p1, const Vec4& color, int entityID = -1);
		static float GetLineThickness();
		static void SetLineThickness(float thickness);

		static void DrawRect(const Vec3& position, const Vec2& size, const Vec4& color, I32 entityID = -1);
		static void DrawRect(const Mat4& transform, const Vec4& color, I32 entityID = -1);

		/** Rotation should be in radians. */
		static void DrawRotatedQuad(const Vec2& position, const Vec2& size, float rotation, const Vec4& color);
		static void DrawRotatedQuad(const Vec3& position, const Vec2& size, float rotation, const Vec4& color);
		static void DrawRotatedQuad(const Vec2& position, const Vec2& size, float rotation, const AssetHandle<Texture2D>& texture, const Vec2& tilingFactor = { 1.0f, 1.0f }, const Vec2& uvOffset = { 0.0f, 0.0f }, const Vec4& tintColor = Vec4(1.0f));
		static void DrawRotatedQuad(const Vec3& position, const Vec2& size, float rotation, const AssetHandle<Texture2D>& texture, const Vec2& tilingFactor = { 1.0f, 1.0f }, const Vec2& uvOffset = { 0.0f, 0.0f }, const Vec4& tintColor = Vec4(1.0f));
		static void DrawRotatedQuad(const Vec2& position, const Vec2& size, float rotation, const Ref<SubTexture2D>& subTexture, const Vec2& tilingFactor = { 1.0f, 1.0f }, const Vec2& uvOffset = { 0.0f, 0.0f }, const Vec4& tintColor = Vec4(1.0f));
		static void DrawRotatedQuad(const Vec3& position, const Vec2& size, float rotation, const Ref<SubTexture2D>& subTexture, const Vec2& tilingFactor = { 1.0f, 1.0f }, const Vec2& uvOffset = { 0.0f, 0.0f }, const Vec4& tintColor = Vec4(1.0f));
	
		static void DrawSprite(const Mat4& transform, const SpriteRendererComponent& spriteComp, I32 entityID);

		static const Renderer2DData& GetRenderer2DData() { return s_Data; }

		static RendererStats& GetStats();
		static void ResetStats();

	private:
		static Renderer2DData s_Data;
	};

}

#include "ZEpch.h"
#include "Engine/Renderer/Renderer2D.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Engine/Renderer/RenderCommand.h"
#include "Engine/Core/Application.h"

namespace ZeoEngine {

	Renderer2DData Renderer2D::s_Data;

	void Renderer2D::Init()
	{
		ZE_PROFILE_FUNCTION();

		s_Data.QuadVertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
		s_Data.QuadVertexPositions[1] = { 0.5f, -0.5f, 0.0f, 1.0f };
		s_Data.QuadVertexPositions[2] = { 0.5f,  0.5f, 0.0f, 1.0f };
		s_Data.QuadVertexPositions[3] = { -0.5f,  0.5f, 0.0f, 1.0f };

		// Quads
		s_Data.QuadVAO = VertexArray::Create();

		s_Data.QuadVBO = VertexBuffer::Create(s_Data.MaxVertices * sizeof(QuadVertex));
		BufferLayout quadLayout = {
			{ ShaderDataType::Float3, "a_Position"     },
			{ ShaderDataType::Float4, "a_Color"        },
			{ ShaderDataType::Float2, "a_TexCoord"     },
			{ ShaderDataType::Float2, "a_TilingFactor" },
			{ ShaderDataType::Float2, "a_UvOffset"     },
			{ ShaderDataType::Float,  "a_TexIndex"     },
			{ ShaderDataType::Int,    "a_EntityID"     },
		};
		s_Data.QuadVBO->SetLayout(quadLayout);
		s_Data.QuadVAO->AddVertexBuffer(s_Data.QuadVBO);

		s_Data.QuadVertexBufferBase = new QuadVertex[s_Data.MaxVertices];

		U32* quadIndices = new U32[s_Data.MaxIndices];
		U32 offset = 0;
		for (U32 i = 0; i < s_Data.MaxIndices; i += 6)
		{
			quadIndices[i + 0] = offset + 0;
			quadIndices[i + 1] = offset + 1;
			quadIndices[i + 2] = offset + 2;

			quadIndices[i + 3] = offset + 2;
			quadIndices[i + 4] = offset + 3;
			quadIndices[i + 5] = offset + 0;

			offset += 4;
		}
		Ref<IndexBuffer> quadIBO = IndexBuffer::Create(quadIndices, s_Data.MaxIndices);
		s_Data.QuadVAO->SetIndexBuffer(quadIBO);
		delete[] quadIndices;

		// Circles
		s_Data.CircleVAO = VertexArray::Create();

		s_Data.CircleVBO = VertexBuffer::Create(s_Data.MaxVertices * sizeof(CircleVertex));
		BufferLayout circleLayout = {
			{ ShaderDataType::Float3, "a_WorldPosition" },
			{ ShaderDataType::Float3, "a_LocalPosition" },
			{ ShaderDataType::Float4, "a_Color"			},
			{ ShaderDataType::Float,  "a_Thickness"		},
			{ ShaderDataType::Float,  "a_Fade"			},
			{ ShaderDataType::Int,    "a_EntityID"		},
		};
		s_Data.CircleVBO->SetLayout(circleLayout);
		s_Data.CircleVAO->AddVertexBuffer(s_Data.CircleVBO);
		s_Data.CircleVAO->SetIndexBuffer(quadIBO); // Use quad IBO
		s_Data.CircleVertexBufferBase = new CircleVertex[s_Data.MaxVertices];

		// Lines
		s_Data.LineVAO = VertexArray::Create();

		s_Data.LineVBO = VertexBuffer::Create(s_Data.MaxVertices * sizeof(LineVertex));
		BufferLayout lineLayout = {
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float4, "a_Color"	   },
			{ ShaderDataType::Int,    "a_EntityID" },
		};
		s_Data.LineVBO->SetLayout(lineLayout);
		s_Data.LineVAO->AddVertexBuffer(s_Data.LineVBO);
		s_Data.LineVertexBufferBase = new LineVertex[s_Data.MaxVertices];

		// Generate a 1x1 white texture to be used by flat color
		s_Data.TextureSlots[0] = Texture2DLibrary::GetWhiteTexture();

		s_Data.QuadShader = Shader::Create("assets/editor/shaders/Renderer2D_Quad.glsl");
		s_Data.CircleShader = Shader::Create("assets/editor/shaders/Renderer2D_Circle.glsl");
		s_Data.LineShader = Shader::Create("assets/editor/shaders/Renderer2D_Line.glsl");
		s_Data.CameraUniformBuffer = UniformBuffer::Create(sizeof(Renderer2DData::CameraData), 0);

	}

	void Renderer2D::Shutdown()
	{
		ZE_PROFILE_FUNCTION();

		delete[] s_Data.QuadVertexBufferBase;
	}

	void Renderer2D::BeginScene(const Camera& camera, const Mat4& transform)
	{
		ZE_PROFILE_FUNCTION();

		s_Data.CameraBuffer.ViewProjection = camera.GetProjection() * glm::inverse(transform);
		s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(Renderer2DData::CameraData));

		StartBatch();
	}

	void Renderer2D::BeginScene(const EditorCamera& camera)
	{
		ZE_PROFILE_FUNCTION();

		s_Data.CameraBuffer.ViewProjection = camera.GetViewProjection();
		s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(Renderer2DData::CameraData));
		StartBatch();
	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera)
	{
		ZE_PROFILE_FUNCTION();

		s_Data.QuadShader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());

		StartBatch();
	}

	void Renderer2D::EndScene()
	{
		ZE_PROFILE_FUNCTION();

		Flush();
	}

	void Renderer2D::Flush()
	{
		if (s_Data.QuadIndexCount)
		{
			s_Data.QuadVBO->Bind();
			auto dataSize = reinterpret_cast<U8*>(s_Data.QuadVertexBufferPtr) - reinterpret_cast<U8*>(s_Data.QuadVertexBufferBase);
			s_Data.QuadVBO->SetData(s_Data.QuadVertexBufferBase, static_cast<U32>(dataSize));

			// Bind textures
			for (U32 i = 0; i < s_Data.TextureSlotIndex; i++)
			{
				s_Data.TextureSlots[i]->SetBindingSlot(i);
				s_Data.TextureSlots[i]->Bind();
			}

			s_Data.QuadShader->Bind();
			RenderCommand::DrawIndexed(s_Data.QuadVAO, s_Data.QuadIndexCount);
			++s_Data.Stats.DrawCalls;
		}

		if (s_Data.CircleIndexCount)
		{
			s_Data.CircleVBO->Bind();
			auto dataSize = reinterpret_cast<U8*>(s_Data.CircleVertexBufferPtr) - reinterpret_cast<U8*>(s_Data.CircleVertexBufferBase);
			s_Data.CircleVBO->SetData(s_Data.CircleVertexBufferBase, static_cast<U32>(dataSize));

			s_Data.CircleShader->Bind();
			RenderCommand::DrawIndexed(s_Data.CircleVAO, s_Data.CircleIndexCount);
			++s_Data.Stats.DrawCalls;
		}

		if (s_Data.LineVertexCount)
		{
			s_Data.LineVBO->Bind();
			auto dataSize = reinterpret_cast<U8*>(s_Data.LineVertexBufferPtr) - reinterpret_cast<U8*>(s_Data.LineVertexBufferBase);
			s_Data.LineVBO->SetData(s_Data.LineVertexBufferBase, static_cast<U32>(dataSize));

			s_Data.LineShader->Bind();
			RenderCommand::SetLineThickness(s_Data.LineThickness);
			RenderCommand::DrawLine(s_Data.LineVAO, s_Data.LineVertexCount);
			++s_Data.Stats.DrawCalls;
		}
	}

	void Renderer2D::StartBatch()
	{
		s_Data.QuadIndexCount = 0;
		s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;

		s_Data.CircleIndexCount = 0;
		s_Data.CircleVertexBufferPtr = s_Data.CircleVertexBufferBase;

		s_Data.LineVertexCount = 0;
		s_Data.LineVertexBufferPtr = s_Data.LineVertexBufferBase;

		s_Data.TextureSlotIndex = 1;

		ResetStats();
	}

	void Renderer2D::NextBatch()
	{
		EndScene();
		StartBatch();
	}

	void Renderer2D::DrawQuad(const Vec2& position, const Vec2& size, const Vec4& color)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, color);
	}

	void Renderer2D::DrawQuad(const Vec3& position, const Vec2& size, const Vec4& color)
	{
		ZE_PROFILE_FUNCTION();

		Mat4 transform = glm::translate(Mat4(1.0f), position) *
			glm::scale(Mat4(1.0f), { size.x, size.y, 1.0f });

		DrawQuad(transform, color);
	}

	void Renderer2D::DrawQuad(const Mat4& transform, const Vec4& color, I32 entityID)
	{
		ZE_PROFILE_FUNCTION();

		if (s_Data.QuadIndexCount >= s_Data.MaxIndices)
		{
			NextBatch();
		}

		constexpr SizeT quadVertexCount = 4;
		constexpr Vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
		// White texture
		constexpr float textureIndex = 0.0f;
		constexpr Vec2 tilingFactor = { 1.0f, 1.0f };
		constexpr Vec2 uvOffset = { 0.0f, 0.0f };

		for (SizeT i = 0; i < quadVertexCount; ++i)
		{
			s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
			s_Data.QuadVertexBufferPtr->Color = color;
			s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[i];
			s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
			s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
			s_Data.QuadVertexBufferPtr->UvOffset = uvOffset;
			s_Data.QuadVertexBufferPtr->EntityID = entityID;
			++s_Data.QuadVertexBufferPtr;
		}

		s_Data.QuadIndexCount += 6;

		++s_Data.Stats.QuadCount;
	}

	void Renderer2D::DrawQuad(const Vec2& position, const Vec2& size, const AssetHandle<Texture2D>& texture, const Vec2& tilingFactor, const Vec2& uvOffset, const Vec4& tintColor)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, texture, tilingFactor, uvOffset, tintColor);
	}

	void Renderer2D::DrawQuad(const Vec3& position, const Vec2& size, const AssetHandle<Texture2D>& texture, const Vec2& tilingFactor, const Vec2& uvOffset, const Vec4& tintColor)
	{
		ZE_PROFILE_FUNCTION();

		Mat4 transform = glm::translate(Mat4(1.0f), position) *
			glm::scale(Mat4(1.0f), { size.x, size.y, 1.0f });

		DrawQuad(transform, texture, tilingFactor, uvOffset, tintColor);
	}

	void Renderer2D::DrawQuad(const Mat4& transform, const AssetHandle<Texture2D>& texture, const Vec2& tilingFactor, const Vec2& uvOffset, const Vec4& tintColor, I32 entityID)
	{
		ZE_PROFILE_FUNCTION();

		if (s_Data.QuadIndexCount >= s_Data.MaxIndices)
		{
			NextBatch();
		}

		float textureIndex = 0.0f;
		for (U32 i = 1; i < s_Data.TextureSlotIndex; ++i)
		{
			if (s_Data.TextureSlots[i] == texture)
			{
				textureIndex = static_cast<float>(i);
				break;
			}
		}
		if (textureIndex == 0.0f)
		{
			if (s_Data.TextureSlotIndex >= Renderer2DData::MaxTextureSlots)
			{
				NextBatch();
			}

			textureIndex = static_cast<float>(s_Data.TextureSlotIndex);
			s_Data.TextureSlots[s_Data.TextureSlotIndex++] = texture;
		}

		constexpr SizeT quadVertexCount = 4;
		constexpr Vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };

		for (SizeT i = 0; i < quadVertexCount; i++)
		{
			s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
			s_Data.QuadVertexBufferPtr->Color = tintColor;
			s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[i];
			s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
			s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
			s_Data.QuadVertexBufferPtr->UvOffset = uvOffset;
			s_Data.QuadVertexBufferPtr->EntityID = entityID;
			++s_Data.QuadVertexBufferPtr;
		}

		s_Data.QuadIndexCount += 6;

		++s_Data.Stats.QuadCount;
	}

	void Renderer2D::DrawQuad(const Vec2& position, const Vec2& size, const Ref<SubTexture2D>& subTexture, const Vec2& tilingFactor, const Vec2& uvOffset, const Vec4& tintColor)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, subTexture, tilingFactor, uvOffset, tintColor);
	}

	void Renderer2D::DrawQuad(const Vec3& position, const Vec2& size, const Ref<SubTexture2D>& subTexture, const Vec2& tilingFactor, const Vec2& uvOffset, const Vec4& tintColor)
	{
		ZE_PROFILE_FUNCTION();

		Mat4 transform = glm::translate(Mat4(1.0f), position) *
			glm::scale(Mat4(1.0f), { size.x, size.y, 1.0f });

		DrawQuad(transform, subTexture, tilingFactor, uvOffset, tintColor);
	}

	void Renderer2D::DrawQuad(const Mat4& transform, const Ref<SubTexture2D>& subTexture, const Vec2& tilingFactor, const Vec2& uvOffset, const Vec4& tintColor, I32 entityID)
	{
		ZE_PROFILE_FUNCTION();

		if (s_Data.QuadIndexCount >= s_Data.MaxIndices)
		{
			NextBatch();
		}

		constexpr SizeT quadVertexCount = 4;
		const Vec2* textureCoords = subTexture->GetTexCoords();
		const Ref<Texture2D>& texture = subTexture->GetTexture();
		float textureIndex = 0.0f;
		for (U32 i = 1; i < s_Data.TextureSlotIndex; ++i)
		{
			if (*s_Data.TextureSlots[i] == *texture)
			{
				textureIndex = static_cast<float>(i);
				break;
			}
		}
		if (textureIndex == 0.0f)
		{
			if (s_Data.TextureSlotIndex >= Renderer2DData::MaxTextureSlots)
			{
				NextBatch();
			}

			textureIndex = static_cast<float>(s_Data.TextureSlotIndex);
			s_Data.TextureSlots[s_Data.TextureSlotIndex++] = AssetHandle<Texture2D>(texture); // TODO:
		}

		for (SizeT i = 0; i < quadVertexCount; i++)
		{
			s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
			s_Data.QuadVertexBufferPtr->Color = tintColor;
			s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[i];
			s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
			s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
			s_Data.QuadVertexBufferPtr->UvOffset = uvOffset;
			s_Data.QuadVertexBufferPtr->EntityID = entityID;
			++s_Data.QuadVertexBufferPtr;
		}

		s_Data.QuadIndexCount += 6;

		++s_Data.Stats.QuadCount;
	}

	void Renderer2D::DrawCircle(const Mat4& transform, const Vec4& color, float thickness, float fade, I32 entityID)
	{
		ZE_PROFILE_FUNCTION();

		//if (s_Data.QuadIndexCount >= s_Data.MaxIndices)
		//{
		//	NextBatch();
		//}

		for (SizeT i = 0; i < 4; ++i)
		{
			s_Data.CircleVertexBufferPtr->WorldPosition = transform * s_Data.QuadVertexPositions[i];
			s_Data.CircleVertexBufferPtr->LocalPosition = s_Data.QuadVertexPositions[i] * 2.0f;
			s_Data.CircleVertexBufferPtr->Color = color;
			s_Data.CircleVertexBufferPtr->Thickness = thickness;
			s_Data.CircleVertexBufferPtr->Fade = fade;
			s_Data.CircleVertexBufferPtr->EntityID = entityID;
			++s_Data.CircleVertexBufferPtr;
		}

		s_Data.CircleIndexCount += 6;

		++s_Data.Stats.QuadCount;
	}

	void Renderer2D::DrawLine(const Vec3& p0, const Vec3& p1, const Vec4& color, int entityID)
	{
		s_Data.LineVertexBufferPtr->Position = p0;
		s_Data.LineVertexBufferPtr->Color = color;
		s_Data.LineVertexBufferPtr->EntityID = entityID;
		++s_Data.LineVertexBufferPtr;

		s_Data.LineVertexBufferPtr->Position = p1;
		s_Data.LineVertexBufferPtr->Color = color;
		s_Data.LineVertexBufferPtr->EntityID = entityID;
		++s_Data.LineVertexBufferPtr;

		s_Data.LineVertexCount += 2;
	}

	float Renderer2D::GetLineThickness()
	{
		return s_Data.LineThickness;
	}

	void Renderer2D::SetLineThickness(float thickness)
	{
		s_Data.LineThickness = thickness;
	}

	void Renderer2D::DrawRect(const Vec3& position, const Vec2& size, const Vec4& color, I32 entityID)
	{
		Vec3 p0 = Vec3(position.x - size.x * 0.5f, position.y - size.y * 0.5f, position.z);
		Vec3 p1 = Vec3(position.x + size.x * 0.5f, position.y - size.y * 0.5f, position.z);
		Vec3 p2 = Vec3(position.x + size.x * 0.5f, position.y + size.y * 0.5f, position.z);
		Vec3 p3 = Vec3(position.x - size.x * 0.5f, position.y + size.y * 0.5f, position.z);
		DrawLine(p0, p1, color);
		DrawLine(p1, p2, color);
		DrawLine(p2, p3, color);
		DrawLine(p3, p0, color);
	}

	void Renderer2D::DrawRect(const Mat4& transform, const Vec4& color, I32 entityID)
	{
		Vec3 lineVertices[4];
		for (U32 i = 0; i < 4; ++i)
		{
			lineVertices[i] = transform * s_Data.QuadVertexPositions[i];
		}
		DrawLine(lineVertices[0], lineVertices[1], color);
		DrawLine(lineVertices[1], lineVertices[2], color);
		DrawLine(lineVertices[2], lineVertices[3], color);
		DrawLine(lineVertices[3], lineVertices[0], color);
	}

	void Renderer2D::DrawRotatedQuad(const Vec2& position, const Vec2& size, float rotation, const Vec4& color)
	{
		DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, color);
	}

	void Renderer2D::DrawRotatedQuad(const Vec3& position, const Vec2& size, float rotation, const Vec4& color)
	{
		ZE_PROFILE_FUNCTION();

		Mat4 transform = glm::translate(Mat4(1.0f), position) *
			glm::rotate(Mat4(1.0f), rotation, { 0.0f, 0.0f, 1.0f }) *
			glm::scale(Mat4(1.0f), { size.x, size.y, 1.0f });

		DrawQuad(transform, color);
	}

	void Renderer2D::DrawRotatedQuad(const Vec2& position, const Vec2& size, float rotation, const AssetHandle<Texture2D>& texture, const Vec2& tilingFactor, const Vec2& uvOffset, const Vec4& tintColor)
	{
		DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, texture, tilingFactor, uvOffset, tintColor);
	}

	void Renderer2D::DrawRotatedQuad(const Vec3& position, const Vec2& size, float rotation, const AssetHandle<Texture2D>& texture, const Vec2& tilingFactor, const Vec2& uvOffset, const Vec4& tintColor)
	{
		ZE_PROFILE_FUNCTION();

		Mat4 transform = glm::translate(Mat4(1.0f), position) *
			glm::rotate(Mat4(1.0f), rotation, { 0.0f, 0.0f, 1.0f }) *
			glm::scale(Mat4(1.0f), { size.x, size.y, 1.0f });

		DrawQuad(transform, texture, tilingFactor, uvOffset, tintColor);
	}

	void Renderer2D::DrawRotatedQuad(const Vec2& position, const Vec2& size, float rotation, const Ref<SubTexture2D>& subTexture, const Vec2& tilingFactor, const Vec2& uvOffset, const Vec4& tintColor)
	{
		DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, subTexture, tilingFactor, uvOffset, tintColor);
	}

	void Renderer2D::DrawRotatedQuad(const Vec3& position, const Vec2& size, float rotation, const Ref<SubTexture2D>& subTexture, const Vec2& tilingFactor, const Vec2& uvOffset, const Vec4& tintColor)
	{
		ZE_PROFILE_FUNCTION();

		Mat4 transform = glm::translate(Mat4(1.0f), position) *
			glm::rotate(Mat4(1.0f), rotation, { 0.0f, 0.0f, 1.0f }) *
			glm::scale(Mat4(1.0f), { size.x, size.y, 1.0f });

		DrawQuad(transform, subTexture, tilingFactor, uvOffset, tintColor);
	}

	void Renderer2D::DrawSprite(const Mat4& transform, const SpriteRendererComponent& spriteComp, I32 entityID)
	{
		if (spriteComp.TextureAsset)
		{
			DrawQuad(transform, spriteComp.TextureAsset, spriteComp.TextureTiling, { 0.0f, 0.0f }, spriteComp.TintColor, entityID);
		}
		else
		{
			DrawQuad(transform, spriteComp.TintColor, entityID);
		}
		
	}

	RendererStats& Renderer2D::GetStats()
	{
		return s_Data.Stats;
	}

	void Renderer2D::ResetStats()
	{
		s_Data.Stats.Reset();
	}

}

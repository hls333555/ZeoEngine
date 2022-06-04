#include "ZEpch.h"
#include "Engine/Renderer/BatchRenderer.h"

#include "Engine/Renderer/VertexArray.h"
#include "Engine/Renderer/Shader.h"
#include "Engine/Renderer/Texture.h"
#include "Engine/Renderer/RenderCommand.h"
#include "Engine/Renderer/Renderer.h"

namespace ZeoEngine {
	
	BatchRenderer::~BatchRenderer()
	{
		delete[] m_PrimitiveBuffer.QuadVertexBufferBase;
	}

	void BatchRenderer::Init()
	{	// Quads
		{
			m_PrimitiveBuffer.QuadVAO = VertexArray::Create();

			m_PrimitiveBuffer.QuadVBO = VertexBuffer::Create(m_PrimitiveBuffer.MaxVertices * sizeof(QuadVertex));
			const BufferLayout quadLayout = {
				{ ShaderDataType::Float3, "a_Position"     },
				{ ShaderDataType::Float4, "a_Color"        },
				{ ShaderDataType::Float2, "a_TexCoord"     },
				{ ShaderDataType::Float2, "a_TilingFactor" },
				{ ShaderDataType::Float2, "a_UvOffset"     },
				{ ShaderDataType::Float,  "a_TexIndex"     },
				{ ShaderDataType::Int,    "a_EntityID"     },
			};
			m_PrimitiveBuffer.QuadVBO->SetLayout(quadLayout);
			m_PrimitiveBuffer.QuadVAO->AddVertexBuffer(m_PrimitiveBuffer.QuadVBO);

			m_PrimitiveBuffer.QuadVertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
			m_PrimitiveBuffer.QuadVertexPositions[1] = { 0.5f, -0.5f, 0.0f, 1.0f };
			m_PrimitiveBuffer.QuadVertexPositions[2] = { 0.5f, 0.5f, 0.0f, 1.0f };
			m_PrimitiveBuffer.QuadVertexPositions[3] = { -0.5f, 0.5f, 0.0f, 1.0f };
			m_PrimitiveBuffer.QuadVertexBufferBase = new QuadVertex[m_PrimitiveBuffer.MaxVertices];

			U32* quadIndices = new U32[m_PrimitiveBuffer.MaxIndices];
			U32 offset = 0;
			for (U32 i = 0; i < m_PrimitiveBuffer.MaxIndices; i += 6)
			{
				quadIndices[i + 0] = offset + 0;
				quadIndices[i + 1] = offset + 1;
				quadIndices[i + 2] = offset + 2;

				quadIndices[i + 3] = offset + 2;
				quadIndices[i + 4] = offset + 3;
				quadIndices[i + 5] = offset + 0;

				offset += 4;
			}
			Ref<IndexBuffer> quadIBO = IndexBuffer::Create(quadIndices, m_PrimitiveBuffer.MaxIndices);
			m_PrimitiveBuffer.QuadVAO->SetIndexBuffer(quadIBO);
			delete[] quadIndices;

			m_PrimitiveBuffer.QuadShader = Shader::Create("assets/editor/shaders/Quad.glsl");
		}

		// Generate a 1x1 white texture to be used by flat color
		m_PrimitiveBuffer.TextureSlots[0] = Texture2DLibrary::GetWhiteTexture();
	}

	void BatchRenderer::StartBatch()
	{
		m_PrimitiveBuffer.QuadIndexCount = 0;
		m_PrimitiveBuffer.QuadVertexBufferPtr = m_PrimitiveBuffer.QuadVertexBufferBase;

		m_PrimitiveBuffer.TextureSlotIndex = 1;
	}

	void BatchRenderer::NextBatch()
	{
		FlushBatch();
		StartBatch();
	}

	void BatchRenderer::FlushBatch() const
	{
		if (m_PrimitiveBuffer.QuadIndexCount)
		{
			m_PrimitiveBuffer.QuadVBO->Bind();
			const auto dataSize = reinterpret_cast<U8*>(m_PrimitiveBuffer.QuadVertexBufferPtr) - reinterpret_cast<U8*>(m_PrimitiveBuffer.QuadVertexBufferBase);
			m_PrimitiveBuffer.QuadVBO->SetData(m_PrimitiveBuffer.QuadVertexBufferBase, static_cast<U32>(dataSize));

			for (U32 i = 0; i < m_PrimitiveBuffer.TextureSlotIndex; i++)
			{
				m_PrimitiveBuffer.TextureSlots[i]->Bind(i);
			}
			m_PrimitiveBuffer.QuadShader->Bind();
			RenderCommand::ToggleDepthWrite(true);
			RenderCommand::DrawIndexed(m_PrimitiveBuffer.QuadVAO, m_PrimitiveBuffer.QuadIndexCount);
		}
	}

	void BatchRenderer::DrawQuad(const Mat4& transform, const Vec4& color, I32 entityID)
	{
		ZE_PROFILE_FUNCTION();

		if (m_PrimitiveBuffer.QuadIndexCount >= m_PrimitiveBuffer.MaxIndices)
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
			m_PrimitiveBuffer.QuadVertexBufferPtr->Position = transform * m_PrimitiveBuffer.QuadVertexPositions[i];
			m_PrimitiveBuffer.QuadVertexBufferPtr->Color = color;
			m_PrimitiveBuffer.QuadVertexBufferPtr->TexCoord = textureCoords[i];
			m_PrimitiveBuffer.QuadVertexBufferPtr->TexIndex = textureIndex;
			m_PrimitiveBuffer.QuadVertexBufferPtr->TilingFactor = tilingFactor;
			m_PrimitiveBuffer.QuadVertexBufferPtr->UvOffset = uvOffset;
			m_PrimitiveBuffer.QuadVertexBufferPtr->EntityID = entityID;
			++m_PrimitiveBuffer.QuadVertexBufferPtr;
		}

		m_PrimitiveBuffer.QuadIndexCount += 6;

		++Renderer::GetStats().QuadCount;
	}

	void BatchRenderer::DrawQuad(const Mat4& transform, const AssetHandle<Texture2D>& texture, const Vec2& tilingFactor, const Vec2& uvOffset, const Vec4& tintColor, I32 entityID)
	{
		ZE_PROFILE_FUNCTION();

		if (m_PrimitiveBuffer.QuadIndexCount >= m_PrimitiveBuffer.MaxIndices)
		{
			NextBatch();
		}

		float textureIndex = 0.0f;
		for (U32 i = 1; i < m_PrimitiveBuffer.TextureSlotIndex; ++i)
		{
			if (m_PrimitiveBuffer.TextureSlots[i] == texture)
			{
				textureIndex = static_cast<float>(i);
				break;
			}
		}
		if (textureIndex == 0.0f)
		{
			if (m_PrimitiveBuffer.TextureSlotIndex >= PrimitiveData::MaxTextureSlots)
			{
				NextBatch();
			}

			textureIndex = static_cast<float>(m_PrimitiveBuffer.TextureSlotIndex);
			m_PrimitiveBuffer.TextureSlots[m_PrimitiveBuffer.TextureSlotIndex++] = texture;
		}

		constexpr SizeT quadVertexCount = 4;
		constexpr Vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };

		for (SizeT i = 0; i < quadVertexCount; i++)
		{
			m_PrimitiveBuffer.QuadVertexBufferPtr->Position = transform * m_PrimitiveBuffer.QuadVertexPositions[i];
			m_PrimitiveBuffer.QuadVertexBufferPtr->Color = tintColor;
			m_PrimitiveBuffer.QuadVertexBufferPtr->TexCoord = textureCoords[i];
			m_PrimitiveBuffer.QuadVertexBufferPtr->TexIndex = textureIndex;
			m_PrimitiveBuffer.QuadVertexBufferPtr->TilingFactor = tilingFactor;
			m_PrimitiveBuffer.QuadVertexBufferPtr->UvOffset = uvOffset;
			m_PrimitiveBuffer.QuadVertexBufferPtr->EntityID = entityID;
			++m_PrimitiveBuffer.QuadVertexBufferPtr;
		}

		m_PrimitiveBuffer.QuadIndexCount += 6;

		++Renderer::GetStats().QuadCount;
	}

}

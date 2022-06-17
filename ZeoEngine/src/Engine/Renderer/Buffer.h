#pragma once

#include <glm/glm.hpp>

#include "Engine/Core/Assert.h"
#include "Engine/Renderer/Bindable.h"
#include "Engine/Renderer/BufferResource.h"
#include "Engine/Renderer/Texture.h"

namespace ZeoEngine {

	enum class ShaderDataType
	{
		None = 0,
		Float, Float2, Float3, Float4,
		Mat3, Mat4,
		Int, Int2, Int3, Int4,
		Bool
	};

	static U32 ShaderDataTypeSize(ShaderDataType type)
	{
		switch (type)
		{
			case ShaderDataType::Float:		return 4;
			case ShaderDataType::Float2:	return 4 * 2;
			case ShaderDataType::Float3:	return 4 * 3;
			case ShaderDataType::Float4:	return 4 * 4;
			case ShaderDataType::Mat3:		return 4 * 3 * 3;
			case ShaderDataType::Mat4:		return 4 * 4 * 4;
			case ShaderDataType::Int:		return 4;
			case ShaderDataType::Int2:		return 4 * 2;
			case ShaderDataType::Int3:		return 4 * 3;
			case ShaderDataType::Int4:		return 4 * 4;
			case ShaderDataType::Bool:		return 1;
			default:
				ZE_CORE_ASSERT(false, "Unknown ShaderDataType!");
				return 0;
		}
	}

	struct BufferElement
	{
		std::string Name;
		ShaderDataType Type;
		U32 Size;
		SizeT Offset;
		bool bNormalized;

		BufferElement(ShaderDataType type, const std::string& name, bool normalized = false)
			: Name(name), Type(type), Size(ShaderDataTypeSize(type)), Offset(0), bNormalized(normalized)
		{
		}

		U32 GetComponentCount() const
		{
			switch (Type)
			{
				case ShaderDataType::Float:		return 1;
				case ShaderDataType::Float2:	return 2;
				case ShaderDataType::Float3:	return 3;
				case ShaderDataType::Float4:	return 4;
				case ShaderDataType::Mat3:		return 3; // 3* float3
				case ShaderDataType::Mat4:		return 4; // 4* float4
				case ShaderDataType::Int:		return 1;
				case ShaderDataType::Int2:		return 2;
				case ShaderDataType::Int3:		return 3;
				case ShaderDataType::Int4:		return 4;
				case ShaderDataType::Bool:		return 1;
					default:
					ZE_CORE_ASSERT(false, "Unknown ShaderDataType!");
					return 0;
			}
		}

	};

	class BufferLayout
	{
	public:
		BufferLayout() = default;
		BufferLayout(std::initializer_list<BufferElement> elements)
			: m_Elements(elements)
		{
			CalculateOffsetAndStride();
		}

		const std::vector<BufferElement>& GetElements() const { return m_Elements; }
		U32 GetStride() const { return m_Stride; }

		std::vector<BufferElement>::iterator begin() { return m_Elements.begin(); }
		std::vector<BufferElement>::iterator end() { return m_Elements.end(); }
		std::vector<BufferElement>::const_iterator begin() const { return m_Elements.begin(); }
		std::vector<BufferElement>::const_iterator end() const { return m_Elements.end(); }

	private:
		void CalculateOffsetAndStride()
		{
			SizeT offset = 0;
			for (auto& element : m_Elements)
			{
				element.Offset = offset;
				offset += element.Size;
				m_Stride += element.Size;
			}
		}

	private:
		std::vector<BufferElement> m_Elements;
		U32 m_Stride = 0;

	};

	class VertexBuffer : public Bindable
	{
	public:
		virtual ~VertexBuffer() = default;

		virtual const BufferLayout& GetLayout() const = 0;
		virtual void SetLayout(const BufferLayout& layout) = 0;

		virtual void SetData(const void* data, U32 size) = 0;

		// Instead of constructor, passing variables to static create fucntion can prevent from casting to different types on class instantiation

		static Ref<VertexBuffer> Create(U32 size);
		static Ref<VertexBuffer> Create(void* vertices, U32 size);

	};

	// Note: Currently ZeoEngine only supports 32-bit index buffers
	class IndexBuffer : public Bindable
	{
	public:
		virtual ~IndexBuffer() = default;

		virtual U32 GetCount() const = 0;

		static Ref<IndexBuffer> Create(U32 count);
		static Ref<IndexBuffer> Create(U32* indices, U32 count);

	};

	struct FrameBufferTextureSpec
	{
		FrameBufferTextureSpec() = default;
		FrameBufferTextureSpec(TextureFormat format, std::vector<SamplerType> samplers = { SamplerType::PointClamp }, U32 textureArraySize = 1)
			: TextureFormat(format), TextureSamplers(std::move(samplers)), TextureArraySize(textureArraySize) {}

		TextureFormat TextureFormat = TextureFormat::None;
		std::vector<SamplerType> TextureSamplers;
		U32 TextureArraySize = 1;
	};

	struct FrameBufferAttachmentSpec
	{
		FrameBufferAttachmentSpec() = default;
		FrameBufferAttachmentSpec(std::initializer_list<FrameBufferTextureSpec> textureSpecs)
			: TextureSpecs(textureSpecs) {}

		std::vector<FrameBufferTextureSpec> TextureSpecs;
	};

	struct FrameBufferSpec
	{
		U32 Width = 1280, Height = 720;
		bool bFixedSize = false;
		FrameBufferAttachmentSpec Attachments;
		U32 Samples = 1;

		bool bSwapChainTarget = false;
	};

	class FrameBuffer : public Bindable, public BufferResource
	{
	public:
		virtual ~FrameBuffer() = default;

		virtual U32 GetFrameBufferID() const = 0;
		virtual const FrameBufferSpec& GetSpec() const = 0;

		virtual const Ref<Texture>& GetColorAttachment(U32 index = 0) const = 0;
		virtual const Ref<Texture>& GetDepthAttachment() const = 0;

		virtual void Resize(U32 width, U32 height) = 0;

		virtual void ReadPixel(U32 attachmentIndex, I32 x, I32 y, void* outPixelData) = 0;

		virtual void ClearColorAttachment(U32 attachmentIndex, I32 clearValue) = 0;
		virtual void ClearColorAttachment(U32 attachmentIndex, const Vec4& clearValue) = 0;

		virtual void BlitColorTo(const Ref<FrameBuffer>& targetFBO, U32 attachmentIndex, U32 targetAttachmentIndex) = 0;
		virtual void BlitDepthTo(const Ref<FrameBuffer>& targetFBO, bool bIncludeStencil = true) = 0;

		virtual void Snapshot(const std::string& imagePath, U32 captureWidth) = 0;

		static Ref<FrameBuffer> Create(const FrameBufferSpec& spec, I32 textureBindingAttachmentIndex = -1, U32 textureBindingSlot = 0);
	};

	// TODO: Refactor when changed to deferred rendering
	namespace UniformBufferBinding
	{
		static constexpr U32 Global = 0;
		static constexpr U32 Camera = 1;
		static constexpr U32 Model = 2;
		static constexpr U32 Light = 3;

		static constexpr U32 ShadowCamera = 4;
		static constexpr U32 Grid = 5;

	}

	class UniformBuffer : public Bindable
	{
	public:
		virtual ~UniformBuffer() = default;

		virtual void SetData(const void* data, U32 size = 0, U32 offset = 0) = 0;

		static Ref<UniformBuffer> Create(U32 size, U32 binding);
	};

}

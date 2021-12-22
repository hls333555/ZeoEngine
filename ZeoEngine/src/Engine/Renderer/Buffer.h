#pragma once

#include <glm/glm.hpp>

#include "Engine/Core/Assert.h"
#include "Engine/Renderer/Bindable.h"
#include "Engine/Renderer/BufferResource.h"

namespace ZeoEngine {

	enum class ShaderDataType
	{
		None = 0,
		Float, Float2, Float3, Float4,
		Mat3, Mat4,
		Int, Int2, Int3, Int4,
		Bool
	};

	static uint32_t ShaderDataTypeSize(ShaderDataType type)
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
		uint32_t Size;
		size_t Offset;
		bool bNormalized;

		BufferElement(ShaderDataType type, const std::string& name, bool normalized = false)
			: Name(name), Type(type), Size(ShaderDataTypeSize(type)), Offset(0), bNormalized(normalized)
		{
		}

		uint32_t GetComponentCount() const
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
		uint32_t GetStride() const { return m_Stride; }

		std::vector<BufferElement>::iterator begin() { return m_Elements.begin(); }
		std::vector<BufferElement>::iterator end() { return m_Elements.end(); }
		std::vector<BufferElement>::const_iterator begin() const { return m_Elements.begin(); }
		std::vector<BufferElement>::const_iterator end() const { return m_Elements.end(); }

	private:
		void CalculateOffsetAndStride()
		{
			size_t offset = 0;
			for (auto& element : m_Elements)
			{
				element.Offset = offset;
				offset += element.Size;
				m_Stride += element.Size;
			}
		}

	private:
		std::vector<BufferElement> m_Elements;
		uint32_t m_Stride = 0;

	};

	class VertexBuffer : public Bindable
	{
	public:
		virtual ~VertexBuffer() = default;

		virtual const BufferLayout& GetLayout() const = 0;
		virtual void SetLayout(const BufferLayout& layout) = 0;

		virtual void SetData(const void* data, uint32_t size) = 0;

		// Instead of constructor, passing variables to static create fucntion can prevent from casting to different types on class instantiation

		static Ref<VertexBuffer> Create(uint32_t size);
		static Ref<VertexBuffer> Create(void* vertices, uint32_t size);

	};

	// Note: Currently ZeoEngine only supports 32-bit index buffers
	class IndexBuffer : public Bindable
	{
	public:
		virtual ~IndexBuffer() = default;

		virtual uint32_t GetCount() const = 0;

		static Ref<IndexBuffer> Create(uint32_t count);
		static Ref<IndexBuffer> Create(uint32_t* indices, uint32_t count);

	};

	enum class FrameBufferTextureFormat
	{
		None = 0,

		// Color
		RGBA8,
		RGBA16F, // This format can store negative values
		RED_INTEGER,

		// Depth
		DEPTH32F,
		// Depth/stencil
		DEPTH24STENCIL8,
	};

	struct FrameBufferTextureSpec
	{
		FrameBufferTextureSpec() = default;
		FrameBufferTextureSpec(FrameBufferTextureFormat format)
			: TextureFormat(format) {}

		FrameBufferTextureFormat TextureFormat = FrameBufferTextureFormat::None;
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
		uint32_t Width = 1280, Height = 720;
		FrameBufferAttachmentSpec Attachments;
		uint32_t Samples = 1;

		bool bSwapChainTarget = false;
	};

	class FrameBuffer : public Bindable, public BufferResource
	{
	public:
		virtual ~FrameBuffer() = default;

		virtual const FrameBufferSpec& GetSpec() const = 0;

		virtual void* GetColorAttachment(uint32_t index = 0) const = 0;
		virtual void* GetDepthAttachment() const = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;

		virtual void ReadPixel(uint32_t attachmentIndex, int32_t x, int32_t y, void* outPixelData) = 0;

		virtual void ClearColorAttachment(uint32_t attachmentIndex, int32_t clearValue) = 0;
		virtual void ClearColorAttachment(uint32_t attachmentIndex, const glm::vec4& clearValue) = 0;

		virtual void Snapshot(const std::string& imagePath, uint32_t captureWidth) = 0;

		virtual void BindAsBuffer() const = 0;
		virtual void UnbindAsBuffer() const = 0;

		static Ref<FrameBuffer> Create(const FrameBufferSpec& spec, int32_t textureBindingAttachmentIndex = -1, uint32_t textureBindingSlot = 0);
	};

	namespace UniformBufferBinding {

		static uint32_t Camera = 0;
		static uint32_t Model = 1;
		static uint32_t Light = 2;
		static uint32_t LightSpace = 3;

		static uint32_t Material = 4;
	}

	class UniformBuffer : public Bindable
	{
	public:
		virtual ~UniformBuffer() = default;

		virtual void SetData(const void* data, uint32_t size = 0, uint32_t offset = 0) = 0;

		static Ref<UniformBuffer> Create(uint32_t size, uint32_t binding);
	};

}

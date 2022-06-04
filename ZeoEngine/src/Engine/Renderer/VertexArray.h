#pragma once

#include "Engine/Renderer/Bindable.h"
#include "Engine/Renderer/Buffer.h"

namespace ZeoEngine {

	class VertexArray : public Bindable
	{
	public:
		virtual ~VertexArray() = default;

		virtual void AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer) = 0;
		virtual void SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer) = 0;
		virtual void SetIndexBufferData(const void* data, U32 size) const = 0;

		virtual const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const = 0;
		virtual const Ref<IndexBuffer>& GetIndexBuffer() const = 0;

		static Ref<VertexArray> Create();

	};
}

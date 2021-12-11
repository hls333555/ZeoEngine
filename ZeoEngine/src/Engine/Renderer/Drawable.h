#pragma once

namespace ZeoEngine {

	class VertexArray;
	class UniformBuffer;

	class Drawable
	{
	public:
		virtual ~Drawable() = default;

		uint32_t GetIndexCount() const { return m_IndexCount; }
		virtual uint32_t GetBaseVertex() const { return 0; }
		virtual uint32_t GetBaseIndex() const { return 0; }

		void Bind() const;
		virtual void Submit() const = 0;

	protected:
		Ref<VertexArray> m_VAO;
		uint32_t m_IndexCount = 0;
		Ref<UniformBuffer> m_ModelUniformBuffer;
	};

}

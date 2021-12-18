#pragma once

namespace ZeoEngine {

	class VertexArray;
	class UniformBuffer;

	class Drawable
	{
	public:
		Drawable(const Ref<VertexArray>& vao, const Ref<UniformBuffer>& ubo);
		virtual ~Drawable() = default;

		virtual uint32_t GetBaseVertex() const { return 0; }
		virtual uint32_t GetBaseIndex() const { return 0; }
		virtual uint32_t GetIndexCount() const = 0;

		void Bind() const;
		virtual void Submit() const = 0;

	private:
		Ref<VertexArray> m_VAO;
		Ref<UniformBuffer> m_ModelUniformBuffer;
	};

}

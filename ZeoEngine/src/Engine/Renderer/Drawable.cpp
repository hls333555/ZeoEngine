#include "ZEpch.h"
#include "Engine/Renderer/Drawable.h"

#include "Engine/Renderer/VertexArray.h"

namespace ZeoEngine {

	Drawable::Drawable(const Ref<VertexArray>& vao, const Ref<UniformBuffer>& ubo)
		: m_VAO(vao), m_ModelUniformBuffer(ubo)
	{
	}

	void Drawable::Bind() const
	{
		m_VAO->Bind();
		m_ModelUniformBuffer->Bind();
	}

}

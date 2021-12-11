#include "ZEpch.h"
#include "Engine/Renderer/Drawable.h"

#include "Engine/Renderer/VertexArray.h"

namespace ZeoEngine {

	void Drawable::Bind() const
	{
		m_VAO->Bind();
		m_ModelUniformBuffer->Bind();
	}

}

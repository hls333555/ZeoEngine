#include "ZEpch.h"
#include "Engine/Renderer/Drawable.h"

#include "Engine/Renderer/VertexArray.h"

namespace ZeoEngine {

	Drawable::Drawable(const Ref<VertexArray>& vao, const Ref<UniformBuffer>& ubo)
		: m_VAO(vao), m_ModelUniformBuffer(ubo)
	{
	}

	void Drawable::AddTechnique(RenderTechnique technique)
	{
		m_Techniques.emplace_back(std::move(technique));
	}

	void Drawable::PrepareTechniques(SizeT size)
	{
		m_Techniques.clear();
		m_Techniques.reserve(size);
	}

	void Drawable::Bind() const
	{
		m_VAO->Bind();
		m_ModelUniformBuffer->Bind();
	}

	void Drawable::Submit()
	{
		for (auto& technique : m_Techniques)
		{
			technique.Submit(*this);
		}
	}

}

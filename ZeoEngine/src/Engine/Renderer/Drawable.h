#pragma once

#include "Engine/Renderer/RenderTechnique.h"

namespace ZeoEngine {

	class VertexArray;
	class UniformBuffer;
	class Scene;

	class Drawable
	{
	public:
		Drawable(const Ref<VertexArray>& vao, const Ref<UniformBuffer>& ubo);
		Drawable(const Drawable&) = delete;
		Drawable(Drawable&&) = default;
		virtual ~Drawable() = default;

		virtual U32 GetBaseVertex() const { return 0; }
		virtual U32 GetBaseIndex() const { return 0; }
		virtual U32 GetIndexCount() const = 0;

		void AddTechnique(RenderTechnique technique);
		void PrepareTechniques(SizeT size);
		void Bind() const;
		void Submit();

	private:
		Ref<VertexArray> m_VAO;
		Ref<UniformBuffer> m_ModelUniformBuffer;
		std::vector<RenderTechnique> m_Techniques;
	};

}

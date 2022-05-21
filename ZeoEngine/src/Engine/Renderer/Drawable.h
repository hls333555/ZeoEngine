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

		virtual uint32_t GetBaseVertex() const { return 0; }
		virtual uint32_t GetBaseIndex() const { return 0; }
		virtual uint32_t GetIndexCount() const = 0;

		void AddTechnique(RenderTechnique technique, const Weak<Scene>& sceneContext);
		void ClearTechniques();
		void Bind() const;
		void Submit();

	private:
		Ref<VertexArray> m_VAO;
		Ref<UniformBuffer> m_ModelUniformBuffer;
		std::vector<RenderTechnique> m_Techniques;
	};

}

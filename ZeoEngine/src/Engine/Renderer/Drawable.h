#pragma once

#include "Engine/Asset/Asset.h"
#include "Engine/Renderer/RenderTechnique.h"

namespace ZeoEngine {

	class VertexArray;
	class UniformBuffer;
	class Material;
	struct SceneContext;

	class Drawable
	{
	public:
		Drawable(const Ref<VertexArray>& vao, const Ref<UniformBuffer>& ubo, const SceneContext* sceneContext);
		Drawable(const Drawable&) = delete;
		Drawable(Drawable&&) = default;
		virtual ~Drawable();

		virtual U32 GetBaseVertex() const { return 0; }
		virtual U32 GetBaseIndex() const { return 0; }
		virtual U32 GetIndexCount() const = 0;
		const SceneContext* GetSceneContext() const { return m_SceneContext; }

		void Bind() const;
		void Submit(AssetHandle materialAsset);

	private:
		void SetMaterial(AssetHandle materialAsset);
		void RebuildStepInstances(const Material& material);

	private:
		Ref<VertexArray> m_VAO;
		Ref<UniformBuffer> m_ModelUniformBuffer;
		AssetHandle m_MaterialAsset;
		const SceneContext* m_SceneContext;

		std::vector<RenderStepInstance> m_StepInstances;
	};

}

#pragma once

#include "Engine/Renderer/SceneRenderer.h"
#include "Engine/Core/Core.h"

namespace ZeoEngine {

	class MaterialEditor;

	class MaterialEditorSceneRenderer : public SceneRenderer
	{
	public:
		explicit MaterialEditorSceneRenderer(const Ref<MaterialEditor>& materialEditor);

	private:
		virtual Scope<RenderGraph> CreateRenderGraph() override;
		virtual Scope<RenderSystemBase> CreateRenderSystem() override;

		virtual void OnRenderScene() override;

	private:
		Ref<MaterialEditor> m_MaterialEditor;
	};

}

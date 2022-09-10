#pragma once

#include "Engine/Renderer/SceneRenderer.h"
#include "Engine/Core/Core.h"

namespace ZeoEngine {

	class LevelPreviewWorld;

	class LevelPreviewSceneRenderer : public SceneRenderer
	{
	public:
		virtual void OnAttach(const Ref<WorldBase>& world) override;

	private:
		virtual Scope<RenderGraph> CreateRenderGraph() override;
		virtual Scope<RenderSystemBase> CreateRenderSystem(const Ref<WorldBase>& world) override;

		virtual void OnRenderScene() override;

	private:
		LevelPreviewWorld* m_World = nullptr;

	};

}

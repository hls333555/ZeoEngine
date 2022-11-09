#pragma once

#include "Engine/Renderer/SceneRenderer.h"
#include "Engine/Core/Core.h"

namespace ZeoEngine {

	class LevelPreviewWorld;

	class LevelPreviewSceneRenderer : public SceneRenderer
	{
	public:
		virtual void OnAttach(WorldBase* world) override;

	private:
		virtual Scope<RenderGraph> CreateRenderGraph() override;
		virtual Scope<RenderSystemBase> CreateRenderSystem(WorldBase* world) override;

		virtual void OnRenderScene() override;

	private:
		LevelPreviewWorld* m_LevelWorld = nullptr;

	};

}

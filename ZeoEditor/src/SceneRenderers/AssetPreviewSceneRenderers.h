#pragma once

#include "Engine/Renderer/SceneRenderer.h"
#include "Engine/Core/Core.h"

namespace ZeoEngine {

	class AssetPreviewSceneRendererBase : public SceneRenderer
	{
	public:
		virtual void OnAttach(WorldBase* world) override;

	private:
		virtual Scope<RenderGraph> CreateRenderGraph() override;

		virtual void OnRenderScene() override;

	private:
		EditorCamera* m_EditorCamera = nullptr;
	};

	class MaterialPreviewSceneRenderer : public AssetPreviewSceneRendererBase
	{
	private:
		virtual Scope<RenderSystemBase> CreateRenderSystem(WorldBase* world) override;
	};

	class MeshPreviewSceneRenderer : public AssetPreviewSceneRendererBase
	{
	private:
		virtual Scope<RenderSystemBase> CreateRenderSystem(WorldBase* world) override;
	};

	class ParticlePreviewSceneRenderer : public AssetPreviewSceneRendererBase
	{
	private:
		virtual Scope<RenderSystemBase> CreateRenderSystem(WorldBase* world) override;
	};

}

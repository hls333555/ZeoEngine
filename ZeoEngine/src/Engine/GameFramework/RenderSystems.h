#pragma once

#include "Engine/GameFramework/Systems.h"

namespace ZeoEngine {

	class SceneRenderer;

	class RenderSystemBase : public ISystem
	{
	public:
		explicit RenderSystemBase(WorldBase* world);

		virtual void OnRenderEditor() = 0;
		virtual void OnRenderRuntime() {}

	protected:
		SceneRenderer* GetSceneRenderer() const { return m_SceneRenderer; }

	private:
		SceneRenderer* m_SceneRenderer = nullptr;
	};

	class RenderSystem : public RenderSystemBase
	{
	public:
		using RenderSystemBase::RenderSystemBase;

		virtual void OnRenderEditor() override;
		virtual void OnRenderRuntime() override;

	private:
		void RenderLights(bool bIsEditor) const;
		void RenderMeshes() const;
		void RenderPhysicsDebug(bool bIsEditor) const;
	};

	class RenderSystem2D : public RenderSystem
	{
	public:
		using RenderSystem::RenderSystem;

		virtual void OnRenderEditor() override;
		virtual void OnRenderRuntime() override;
	};

	class ParticlePreviewRenderSystem : public RenderSystemBase
	{
	public:
		using RenderSystemBase::RenderSystemBase;

		virtual void OnRenderEditor() override;
	};

	class MeshPreviewRenderSystem : public RenderSystemBase
	{
	public:
		using RenderSystemBase::RenderSystemBase;

		virtual void OnRenderEditor() override;
	};

	class MaterialPreviewRenderSystem : public RenderSystemBase
	{
	public:
		using RenderSystemBase::RenderSystemBase;

		virtual void OnRenderEditor() override;
	};
	
}

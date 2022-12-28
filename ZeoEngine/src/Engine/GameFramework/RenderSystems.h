#pragma once

#include "Engine/GameFramework/Systems.h"

namespace ZeoEngine {

	class SceneRenderer;

	class RenderSystemBase : public ISystem
	{
	public:
		explicit RenderSystemBase(WorldBase* world);

		virtual void OnRenderEditor(bool bIsAssetPreview) = 0;
		virtual void OnRenderRuntime() {}

	protected:
		SceneRenderer* GetSceneRenderer() const { return m_SceneRenderer; }

	private:
		SceneRenderer* m_SceneRenderer = nullptr;
	};

	class BillboardRenderSystem : public RenderSystemBase
	{
	public:
		using RenderSystemBase::RenderSystemBase;

		virtual void OnRenderEditor(bool bIsAssetPreview) override;
	};

	class CameraVisualizerRenderSystem : public RenderSystemBase
	{
	public:
		using RenderSystemBase::RenderSystemBase;

		virtual void OnRenderEditor(bool bIsAssetPreview) override;
	};

	class MeshRenderSystem : public RenderSystemBase
	{
	public:
		using RenderSystemBase::RenderSystemBase;

		virtual void OnRenderEditor(bool bIsAssetPreview) override;
		virtual void OnRenderRuntime() override;
	};

	class DirectionalLightRenderSystem : public RenderSystemBase
	{
	public:
		using RenderSystemBase::RenderSystemBase;

		virtual void OnRenderEditor(bool bIsAssetPreview) override;
		virtual void OnRenderRuntime() override;
	};

	class PointLightRenderSystem : public RenderSystemBase
	{
	public:
		using RenderSystemBase::RenderSystemBase;

		virtual void OnRenderEditor(bool bIsAssetPreview) override;
		virtual void OnRenderRuntime() override;
	};

	class SpotLightRenderSystem : public RenderSystemBase
	{
	public:
		using RenderSystemBase::RenderSystemBase;

		virtual void OnRenderEditor(bool bIsAssetPreview) override;
		virtual void OnRenderRuntime() override;
	};

	class PhysicsDebugRenderSystem : public RenderSystemBase
	{
	public:
		using RenderSystemBase::RenderSystemBase;

		virtual void OnRenderEditor(bool bIsAssetPreview) override;
		virtual void OnRenderRuntime() override;

	private:
		void DrawColliders(bool bDraw);
		void DrawWorldBounds() const;
	};

	class ParticleSystemRenderSystem : public RenderSystemBase
	{
	public:
		using RenderSystemBase::RenderSystemBase;

		virtual void OnRenderEditor(bool bIsAssetPreview) override;
		virtual void OnRenderRuntime() override;
	};

	class SpriteRenderSystem : public RenderSystemBase
	{
	public:
		using RenderSystemBase::RenderSystemBase;

		virtual void OnRenderEditor(bool bIsAssetPreview) override;
		virtual void OnRenderRuntime() override;
	};

	class CircleRenderSystem : public RenderSystemBase
	{
	public:
		using RenderSystemBase::RenderSystemBase;

		virtual void OnRenderEditor(bool bIsAssetPreview) override;
		virtual void OnRenderRuntime() override;
	};

	class ParticlePreviewRenderSystem : public RenderSystemBase
	{
	public:
		using RenderSystemBase::RenderSystemBase;

		virtual void OnRenderEditor(bool bIsAssetPreview) override;
	};

	class MeshPreviewRenderSystem : public RenderSystemBase
	{
	public:
		using RenderSystemBase::RenderSystemBase;

		virtual void OnRenderEditor(bool bIsAssetPreview) override;
	};
	
}

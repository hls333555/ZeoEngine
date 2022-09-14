#pragma once

#include "Engine/GameFramework/World.h"
#include "Engine/Core/DeltaTime.h"
#include "Engine/Events/Event.h"

class b2World;

namespace ZeoEngine {

	class SceneRenderer;
	class SceneCamera;

	class ISystem
	{
	public:
		explicit ISystem(const Ref<WorldBase>& world);
		virtual ~ISystem() = default;

	protected:
		Ref<WorldBase> GetWorld() const { return m_World.lock(); }
		Ref<Scene> GetScene() const { return GetWorld()->GetActiveScene(); }

	private:
		Weak<WorldBase> m_World;
	};

	class SystemBase : public ISystem
	{
	public:
		explicit SystemBase(const Ref<WorldBase>& world);

		virtual void OnCreate() {}
		void OnUpdate(DeltaTime dt) const;

		void BindUpdateFuncToEditor();
		void BindUpdateFuncToRuntime();

		virtual void OnRuntimeStart() {}
		virtual void OnRuntimeStop() {}
	private:
		virtual void OnUpdateEditor(DeltaTime dt) {}
		virtual void OnUpdateRuntime(DeltaTime dt) {}

	private:
		entt::delegate<void(DeltaTime)> m_UpdateFuncDel;
	};

	class RenderSystemBase : public ISystem
	{
	public:
		explicit RenderSystemBase(const Ref<WorldBase>& world);
		
		virtual void OnRenderEditor() = 0;
		virtual void OnRenderRuntime() {}

		std::pair<SceneCamera*, Mat4> GetActiveSceneCamera() const;

	protected:
		Ref<SceneRenderer> GetSceneRenderer() const { return m_SceneRenderer.lock(); }

	private:
		Weak<SceneRenderer> m_SceneRenderer;
	};

	class ParticleUpdateSystem : public SystemBase
	{
	public:
		using SystemBase::SystemBase;

		virtual void OnUpdateEditor(DeltaTime dt) override;
		virtual void OnUpdateRuntime(DeltaTime dt) override;

	private:
		virtual void OnUpdateImpl(DeltaTime dt);
	};

	class ParticlePreviewUpdateSystem : public ParticleUpdateSystem
	{
	public:
		using ParticleUpdateSystem::ParticleUpdateSystem;

	private:
		virtual void OnUpdateImpl(DeltaTime dt) override;
	};

	class ScriptSystem : public SystemBase
	{
	public:
		using SystemBase::SystemBase;

		virtual void OnUpdateRuntime(DeltaTime dt) override;

		virtual void OnRuntimeStart() override;
		virtual void OnRuntimeStop() override;
	};

	class NativeScriptSystem : public SystemBase
	{
	public:
		using SystemBase::SystemBase;

		virtual void OnUpdateRuntime(DeltaTime dt) override;
		void OnEvent(Event& e);
	};

	class PhysicsSystem : public SystemBase
	{
	public:
		using SystemBase::SystemBase;

		virtual void OnUpdateRuntime(DeltaTime dt) override;

		virtual void OnRuntimeStart() override;
		virtual void OnRuntimeStop() override;
	};

	class PhysicsSystem2D : public PhysicsSystem
	{
	public:
		using PhysicsSystem::PhysicsSystem;

		virtual void OnUpdateRuntime(DeltaTime dt) override;

		virtual void OnRuntimeStart() override;
		virtual void OnRuntimeStop() override;

	private:
		b2World* m_PhysicsWorld = nullptr;
	};

	class RenderSystem : public RenderSystemBase
	{
	public:
		using RenderSystemBase::RenderSystemBase;

		virtual void OnRenderEditor() override;
		virtual void OnRenderRuntime() override;

	private:
		void RenderLights(bool bIsEditor);
		void RenderMeshes();
	};

	class RenderSystem2D : public RenderSystem
	{
	public:
		using RenderSystem::RenderSystem;

		virtual void OnRenderEditor() override;
		virtual void OnRenderRuntime() override;

	private:
		void OnRenderImpl();
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

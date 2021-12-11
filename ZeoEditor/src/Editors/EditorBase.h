#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Core/DeltaTime.h"
#include "Engine/GameFramework/Entity.h"

namespace ZeoEngine {

	class EditorCamera;
	class EditorUIRendererBase;
	class Scene;
	class FrameBuffer;
	class IAsset;
	class RenderGraph;

	class EditorBase : public std::enable_shared_from_this<EditorBase>
	{
	public:
		EditorBase() = delete;
		explicit EditorBase(const char* editorName);
		~EditorBase();

	protected:
		template<typename Derived>
		Ref<Derived> SharedFromBase()
		{
			return std::static_pointer_cast<Derived>(shared_from_this());
		}
		template<typename Derived>
		Ref<const Derived> SharedFromBase() const
		{
			return std::static_pointer_cast<const Derived>(shared_from_this());
		}

	private:
		using std::enable_shared_from_this<EditorBase>::shared_from_this;

	public:
		virtual void OnAttach();
		virtual void OnDetach() {}
		void OnUpdate(DeltaTime dt);
		void OnImGuiRender();
		void OnEvent(Event& e);

		const std::string& GetEditorName() const { return m_EditorName; }

		bool* GetShowPtr() { return &m_bShow; }

		Entity GetContextEntity() const { return m_ContextEntity; }
		void SetContextEntity(Entity entity) { m_ContextEntity = entity; }
		void FocusContextEntity();

		EditorCamera* GetEditorCamera() const { return m_EditorCamera; }
		void SetEditorCamera(EditorCamera* camera) { m_EditorCamera = camera; }

		const Ref<EditorUIRendererBase>& GetEditorUIRenderer() const { return m_EditorUIRenderer; }

		const Ref<Scene>& GetScene() const { return m_ActiveScene; }
		template<typename T>
		Ref<T> GetScene() const
		{
			return std::dynamic_pointer_cast<T>(m_ActiveScene);
		}
		void SetActiveScene(const Ref<Scene>& newScene) { m_ActiveScene = newScene; }

		const Ref<FrameBuffer>& GetFrameBuffer() const { return m_FBO; }
		const Scope<RenderGraph>& GetRenderGraph() const { return m_RenderGraph; }

		void Open();

		/** Create an empty scene. */
		void NewScene(bool bIsFromLoad = false);
		void LoadScene();
		/** Create an empty scene and load asset from disk. */
		void LoadScene(const std::string& path);
		void SaveScene();
		void SaveScene(const std::string& path);
		void SaveSceneAs();

		void BlockSceneEvents(bool bBlock) { m_bBlockSceneEvents = bBlock; }

	private:
		virtual Ref<EditorUIRendererBase> CreateEditorUIRenderer() = 0;
		virtual Ref<Scene> CreateScene() = 0;

	public:
		virtual AssetHandle<IAsset> GetAsset() const = 0;
		virtual AssetTypeId GetAssetTypeId() const = 0;
	private:
		virtual void LoadAsset(const std::string& path) = 0;
		virtual void SaveAsset(const std::string& path) = 0;

		virtual Ref<FrameBuffer> CreateFrameBuffer() = 0;
		virtual Scope<RenderGraph> CreateRenderGraph(const Ref<FrameBuffer>& fbo) = 0;

	public:
		/**
		 * Called before scene being created.
		 * The bool argument indicates whether the scene is created by "Load" or "New"
		 */
		entt::sink<void(bool)> m_PreSceneCreate{ m_PreSceneCreateDel };
		/**
		 * Called after scene being created.
		 * The bool argument indicates whether the scene is created by "Load" or "New"
		 */
		entt::sink<void(bool)> m_PostSceneCreate{ m_PostSceneCreateDel };
		/**
		 * Called after scene being loaded.
		 * All data have been loaded, initialization stuff can be done here (e.g. particle system creation)
		 */
		entt::sink<void()> m_PostSceneLoad{ m_PostSceneLoadDel };
		/** Called after scene being rendered. */
		entt::sink<void(const Ref<FrameBuffer>&)> m_PostSceneRender{ m_PostSceneRenderDel };
		
	private:
		std::string m_EditorName;

		bool m_bShow = true;

		Entity m_ContextEntity;
		EditorCamera* m_EditorCamera = nullptr;

		Ref<EditorUIRendererBase> m_EditorUIRenderer;
		Ref<Scene> m_ActiveScene;

		Ref<FrameBuffer> m_FBO;
		Scope<RenderGraph> m_RenderGraph;

		bool m_bBlockSceneEvents = true;

		entt::sigh<void(bool)> m_PreSceneCreateDel, m_PostSceneCreateDel;
		entt::sigh<void()> m_PostSceneLoadDel;
		entt::sigh<void(const Ref<FrameBuffer>&)> m_PostSceneRenderDel;
	};

}

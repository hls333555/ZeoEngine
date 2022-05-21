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
	class SceneRenderer;

	class EditorBase : public std::enable_shared_from_this<EditorBase>
	{
		friend class EditorViewPanelBase;

	public:
		EditorBase() = delete;
		explicit EditorBase(const char* editorName);
		virtual ~EditorBase();

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
		void OnUpdate(DeltaTime dt) const;
		void OnImGuiRender() const;
		void OnEvent(Event& e) const;

		const std::string& GetEditorName() const { return m_EditorName; }

		bool* GetShowPtr() { return &m_bShow; }

		Entity GetContextEntity() const { return m_ContextEntity; }
		void SetContextEntity(Entity entity) { m_ContextEntity = entity; }
		void FocusContextEntity() const;

		EditorCamera* GetEditorCamera() const { return m_EditorCamera; }
		void SetEditorCamera(EditorCamera* camera) { m_EditorCamera = camera; }

		const Ref<EditorUIRendererBase>& GetEditorUIRenderer() const { return m_EditorUIRenderer; }
		const Ref<SceneRenderer>& GetSceneRenderer() const { return m_SceneRenderer; }

		const Ref<FrameBuffer>& GetFrameBuffer() const;

		const Ref<Scene>& GetScene() const { return m_ActiveScene; }
		template<typename T>
		Ref<T> GetScene() const
		{
			return std::dynamic_pointer_cast<T>(m_ActiveScene);
		}
		void SetActiveScene(const Ref<Scene>& newScene, bool bIsCreateDefault);

		void NewDefaultScene();
		void NewScene(bool bIsCreateDefault);
		void LoadScene();
		/** Create an empty scene and load asset from disk. */
		void LoadScene(const std::string& path);
		void SaveScene();
		void SaveScene(const std::string& path);
		void SaveSceneAs();

		void Open();

		void BlockSceneEvents(bool bBlock) { m_bBlockSceneEvents = bBlock; }

	private:
		virtual Ref<EditorUIRendererBase> CreateEditorUIRenderer() = 0;
		virtual Ref<Scene> CreateScene() = 0;
		virtual Ref<SceneRenderer> CreateSceneRenderer() = 0;

		void PostSceneRender(const Ref<FrameBuffer>& fbo) const;

		void OnActiveSceneChanged(const Ref<Scene>& scene, bool bIsCreateDefault);

	public:
		virtual AssetHandle<IAsset> GetAsset() const = 0;
		virtual AssetTypeId GetAssetTypeId() const = 0;
	private:
		virtual void LoadAsset(const std::string& path) = 0;
		virtual void SaveAsset(const std::string& path);
		virtual void LoadAndApplyDefaultAsset() = 0;

		virtual Entity CreatePreviewEntity(const Ref<Scene>& scene) { return {}; }

	public:
		/**
		 * Called before scene being created.
		 * The bool argument indicates whether the scene is created as a default empty scene
		 */
		entt::sink<entt::sigh<void(bool)>> m_PreSceneCreate{ m_PreSceneCreateDel };
		/**
		 * Called after scene being created.
		 * The bool argument indicates whether the scene is created as a default empty scene
		 */
		entt::sink<entt::sigh<void(const Ref<Scene>&, bool)>> m_PostSceneCreate{ m_PostSceneCreateDel };
		/**
		 * Called when active scene has been changed.
		 * The bool argument indicates whether the scene is created as a default empty scene
		 */
		entt::sink<entt::sigh<void(const Ref<Scene>&, bool)>> m_OnActiveSceneChanged{ m_OnActiveSceneChangedDel };
		/**
		 * Called after scene being loaded.
		 * All data have been loaded, initialization stuff can be done here (e.g. particle system creation)
		 */
		entt::sink<entt::sigh<void()>> m_PostSceneLoad{ m_PostSceneLoadDel };
		/** Called after scene being rendered. */
		entt::sink<entt::sigh<void(const Ref<FrameBuffer>&)>> m_PostSceneRender{ m_PostSceneRenderDel };
		/** Called when view panel being resized. */
		entt::sink<entt::sigh<void(uint32_t, uint32_t)>> m_OnViewportResize{ m_OnViewportResizeDel };
		
	private:
		std::string m_EditorName;

		bool m_bShow = true;

		/** For level editor, it is the selected entity; for others, it is the preview entity */
		Entity m_ContextEntity;
		EditorCamera* m_EditorCamera = nullptr;

		// TODO: Change to Scope
		Ref<EditorUIRendererBase> m_EditorUIRenderer;
		Ref<Scene> m_ActiveScene;
		Ref<SceneRenderer> m_SceneRenderer;

		bool m_bBlockSceneEvents = true;

		entt::sigh<void(bool)> m_PreSceneCreateDel;
		entt::sigh<void(const Ref<Scene>&, bool)> m_PostSceneCreateDel;
		entt::sigh<void(const Ref<Scene>&, bool)> m_OnActiveSceneChangedDel;
		entt::sigh<void()> m_PostSceneLoadDel;
		entt::sigh<void(const Ref<FrameBuffer>&)> m_PostSceneRenderDel;
		entt::sigh<void(uint32_t, uint32_t)> m_OnViewportResizeDel;
	};

}

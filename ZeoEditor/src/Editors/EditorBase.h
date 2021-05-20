#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Core/DeltaTime.h"
#include "Core/EditorTypes.h"
#include "Engine/Core/EngineTypes.h"
#include "Engine/GameFramework/Entity.h"

namespace ZeoEngine {

	class EditorCamera;
	class DockspaceBase;
	class Scene;
	class FrameBuffer;

	class EditorBase
	{
		friend class EditorManager;

	private:
		EditorBase() = delete;
		explicit EditorBase(EditorType type);
	protected:
		virtual ~EditorBase();

	public:
		virtual void OnAttach();
		virtual void OnDetach() {}
		void OnUpdate(DeltaTime dt);
		void OnImGuiRender();
		void OnEvent(Event& e);

		EditorType GetEditorType() const { return m_EditorType; }

		bool* GetShowPtr() { return &m_bShow; }

		Entity GetContextEntity() const { return m_ContextEntity; }
		void SetContextEntity(Entity entity) { m_ContextEntity = entity; }
		void FocusContextEntity();

		void SetEditorCamera(EditorCamera* camera) { m_EditorCamera = camera; }

		DockspaceBase* GetDockspace() const { return m_Dockspace; }
		const Ref<Scene>& GetScene() const { return m_Scene; }
		const Ref<FrameBuffer>& GetFrameBuffer() const { return m_FBO; }

		void Open();

		/** Create an empty scene and init camera. */
		void CreateNewScene(bool bIsFromOpenScene = false);
		void OpenScene();
		void OpenScene(const std::string& path);
		void SaveScene();
		void SaveSceneAs();

		void BlockSceneEvents(bool bBlock) { m_bBlockSceneEvents = bBlock; }

	private:
		virtual AssetType GetAssetType() const = 0;
		virtual void Serialize(const std::string& filePath) {}
		virtual void Deserialize(const std::string& filePath) {}

		void CreateDockspace();

		void CreateScene();

		void CreateFrameBuffer();
		void BeginFrameBuffer();
		void EndFrameBuffer();

	public:
		/**
		 * Called before scene is created.
		 * The bool argument indicates whether the scene is created by "OpenScene" or "NewScene"
		 */
		entt::sink<void(bool)> m_PreSceneCreate{ m_PreSceneCreateDel };
		/**
		 * Called after scene is created.
		 * The bool argument indicates whether the scene is created by "OpenScene" or "NewScene"
		 */
		entt::sink<void(bool)> m_PostSceneCreate{ m_PostSceneCreateDel };
		/** Called after scene is rendered. */
		entt::sink<void(const Ref<FrameBuffer>&)> m_PostSceneRender{ m_PostSceneRenderDel };

	private:
		EditorType m_EditorType;

		bool m_bShow = true;

		Entity m_ContextEntity;
		EditorCamera* m_EditorCamera = nullptr;

		DockspaceBase* m_Dockspace = nullptr;
		Ref<Scene> m_Scene;
		Ref<FrameBuffer> m_FBO;

		bool m_bBlockSceneEvents = true;

		entt::sigh<void(bool)> m_PreSceneCreateDel, m_PostSceneCreateDel;
		entt::sigh<void(const Ref<FrameBuffer>&)> m_PostSceneRenderDel;
	};

}

#pragma once

#include "Core/WindowManager.h"
#include "Engine/GameFramework/Entity.h"
#include "Engine/Renderer/Buffer.h"
#include "Engine/Core/EngineTypes.h"

namespace ZeoEngine {

	class EditorCamera;

	class DockspaceBase
	{
		friend class DockspaceManager;

	private:
		DockspaceBase() = delete;
		DockspaceBase(const DockspaceSpec& spec);
	protected:
		virtual ~DockspaceBase() = default;

	public:
		virtual void OnAttach();
		virtual void OnDetach() {}
		void OnUpdate(DeltaTime dt);
		void OnImGuiRender();
		void OnEvent(Event& e);

		bool* GetShowPtr() { return &m_bShow; }
		bool IsDockspaceFocused() const { return m_bIsDockspaceFocused; }
		bool IsDockspaceHovered() const { return m_bIsDockspaceHovered; }

		const Ref<Scene>& GetScene() const { return m_Scene; }
		const Ref<FrameBuffer>& GetFrameBuffer() const { return m_FBO; }

		virtual PanelType GetViewportPanelType() const = 0;

		Entity GetContextEntity() const { return m_ContextEntity; }
		void SetContextEntity(Entity entity) { m_ContextEntity = entity; }
		void FocusContextEntity();

		void SetEditorCamera(EditorCamera* camera) { m_EditorCamera = camera; }

	protected:
		EditorMenu& CreateMenu(const std::string& menuName);

		void CreatePanel(PanelType panelType);

	public:
		PanelBase* TogglePanel(PanelType panelType, bool bOpen);
		template<typename T = PanelBase>
		T* GetPanel(PanelType panelType)
		{
			return dynamic_cast<T*>(m_PanelManager.GetPanel(panelType));
		}

		void BlockSceneEvents(bool bBlock) { m_bBlockSceneEvents = bBlock; }

		/** Create an empty scene and init camera. */
		void CreateNewScene(bool bIsFromOpenScene = false);
		void OpenScene();
		void SaveScene();
		void SaveSceneAs();

	private:
		virtual AssetType GetAssetType() const { return AssetType::NONE; }
		virtual void Serialize(const std::string& filePath) {}
		virtual void Deserialize(const std::string& filePath) {}

		void CreateScene();

		void CreateFrameBuffer();
		void BeginFrameBuffer();
		void EndFrameBuffer();

		virtual void PreRenderDockspace();
		void RenderDockspace();
		virtual void BuildDockWindows(ImGuiID dockspaceID) = 0;

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
		DockspaceSpec m_DockspaceSpec;

		bool m_bShow = true;
		bool m_bIsDockspaceFocused = false, m_bIsDockspaceHovered = false;
		bool m_bBlockSceneEvents = true;
		bool m_bShouldRebuildDockLayout = false;

		Entity m_ContextEntity;
		EditorCamera* m_EditorCamera = nullptr;

		Ref<Scene> m_Scene;
		Ref<FrameBuffer> m_FBO;
		MenuManager m_MenuManager;
		PanelManager m_PanelManager;

		entt::sigh<void(bool)> m_PreSceneCreateDel, m_PostSceneCreateDel;
		entt::sigh<void(const Ref<FrameBuffer>&)> m_PostSceneRenderDel;

	};

}

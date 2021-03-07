#pragma once

#include "Core/WindowManager.h"
#include "Engine/GameFramework/Entity.h"
#include "Engine/GameFramework/Scene.h"
#include "Engine/Renderer/Buffer.h"
#include "Engine/Utils/PlatformUtils.h"

namespace ZeoEngine {

	class EditorCamera;

	class EditorDockspace
	{
		friend class DockspaceManager;

	private:
		EditorDockspace() = delete;
		EditorDockspace(const EditorDockspaceSpec& spec);
	public:
		virtual ~EditorDockspace() = default;

		virtual void OnAttach();
		virtual void OnDetach() {}
		void OnUpdate(DeltaTime dt);
		void OnImGuiRender();
		void OnEvent(Event& e);

		bool* GetShowPtr() { return &m_bShow; }
		bool IsDockspaceFocused() const { return m_bIsDockspaceFocused; }
		bool IsDockspaceHovered() const { return m_bIsDockspaceHovered; }
		Entity GetContextEntity() const { return m_ContextEntity; }
		void SetContextEntity(Entity newEntity) { m_ContextEntity = newEntity; }
		const Ref<Scene>& GetScene() const { return m_Scene; }
		const Ref<FrameBuffer>& GetFrameBuffer() const { return m_FBO; }
		virtual EditorPanelType GetViewportPanelType() const = 0;
		EditorCamera* GetEditorCamera() const { return m_EditorCamera; }
		void SetEditorCamera(EditorCamera* camera) { m_EditorCamera = camera; }

		EditorMenu& CreateMenu(const std::string& menuName);

		void OpenPanel(EditorPanelType panelType);
		void CreatePanel(EditorPanelType panelType);
		template<typename T = EditorPanel>
		T* GetPanel(EditorPanelType panelType)
		{
			return dynamic_cast<T*>(m_PanelManager.GetPanel(panelType));
		}

		void BlockEvents(bool bBlock) { m_bBlockEvents = bBlock; }

		/** Create an empty scene and init camera. */
		virtual void CreateNewScene(bool bIsFromOpenScene = false);
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

		virtual void PostRenderScene(const Ref<FrameBuffer>& frameBuffer) {}
		virtual int32_t PreRenderDockspace();

		void RenderDockspace();
		virtual void BuildDockWindows(ImGuiID dockspaceID) = 0;

	protected:
		EditorDockspaceSpec m_DockspaceSpec;
		Entity m_ContextEntity;
	private:
		entt::sigh<void(bool)> m_OnSceneCreateDel;
	public:
		/**
		 * Called when scene is created.
		 * The bool argument indicates whether the scene is created by "OpenScene" or "NewScene"
		 */
		entt::sink<void(bool)> m_OnSceneCreate{ m_OnSceneCreateDel };
	private:
		bool m_bShow = true;

		bool m_bIsDockspaceFocused = false, m_bIsDockspaceHovered = false;
		bool m_bBlockEvents = true;

		EditorCamera* m_EditorCamera;

		Ref<Scene> m_Scene;
		Ref<FrameBuffer> m_FBO;
		MenuManager m_MenuManager;
		PanelManager m_PanelManager;

		bool m_bShouldRebuildDockLayout = false;
	};

}

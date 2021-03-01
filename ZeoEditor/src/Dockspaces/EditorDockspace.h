#pragma once

#include <string>
#include <unordered_map>

#include "Engine/GameFramework/Entity.h"
#include "Engine/GameFramework/Scene.h"
#include "Engine/Renderer/Buffer.h"
#include "Menus/EditorMenu.h"
#include "Panels/EditorPanel.h"
#include "Engine/Events/Event.h"
#include "Engine/ImGui/MyImGui.h"
#include "Utils/EditorUtils.h"
#include "Engine/Utils/PlatformUtils.h"

namespace ZeoEngine {

	class EditorLayer;
	class EditorCamera;

	class EditorDockspace
	{
		friend class DockspaceManager;
		friend class ScenePanel;
		friend class MainDockspace;

	public:
		EditorDockspace() = delete;
		EditorDockspace(EditorDockspaceType dockspaceType, EditorLayer* context, bool bDefaultShow = false,
			const glm::vec2& dockspacePadding = glm::vec2(0.0f, 0.0f),
			ImGuiWindowFlags dockspaceWindowFlags = ImGuiWindowFlags_MenuBar,
			ImVec2Data initialSize = ImVec2Data::DefaultSize, ImVec2Data initialPos = ImVec2Data::DefaultPos);

	public:
		virtual void OnAttach();
		virtual void OnDetach() {}
		void OnUpdate(DeltaTime dt);
		void OnImGuiRender();
		void OnEvent(Event& e);

		EditorDockspaceType GetDockspaceType() const { return m_DockspaceType; }
		std::string GetDockspaceName() const { return std::move(ResolveEditorNameFromEnum(m_DockspaceType)); }
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

	protected:
		void PushDockspace(EditorDockspace* dockspace);
	public:
		void PushMenu(EditorMenu* menu);
		void PushPanel(EditorPanel* panel);
		template<typename T = EditorPanel>
		T* GetPanelByType(EditorPanelType panelType)
		{
			return dynamic_cast<T*>(m_PanelManager.GetPanelByName(ResolveEditorNameFromEnum(panelType)));
		}

		EditorDockspace* OpenEditor(EditorDockspaceType dockspaceType);

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

		void CreateFrameBuffers();
		void BeginFrameBuffer();
		void EndFrameBuffer();

		virtual void PostSceneRender() {}

		void RenderDockspace();
		virtual void BuildDockWindows(ImGuiID dockspaceID) = 0;

	protected:
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
		EditorDockspaceType m_DockspaceType;
		ImVec2Data m_InitialPos, m_InitialSize;
		glm::vec2 m_DockspacePadding;
		ImGuiWindowFlags m_DockspaceWindowFlags;
		EditorLayer* m_EditorContext;
		bool m_bShow;
		bool m_bIsDockspaceFocused = false, m_bIsDockspaceHovered = false;
		bool m_bBlockEvents = true;

		EditorCamera* m_EditorCamera;

		Ref<Scene> m_Scene;
		Ref<FrameBuffer> m_FBO;
		MenuManager m_MenuManager;
		PanelManager m_PanelManager;

		bool m_bShouldRebuildDockLayout{ false };
	};

	class DockspaceManager
	{
	public:
		DockspaceManager() = default;
		~DockspaceManager();

		void OnUpdate(DeltaTime dt);
		void OnImGuiRender();
		void OnEvent(Event& e);

		void PushDockspace(EditorDockspace* dockspace);
		EditorDockspace* GetDockspaceByName(const std::string& dockspaceName);

		void RebuildDockLayout(const std::string& dockspaceName);

	private:
		std::unordered_map<std::string, EditorDockspace*> m_Dockspaces;
	};

}
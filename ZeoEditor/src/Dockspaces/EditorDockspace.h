#pragma once

#include <string>
#include <unordered_map>
#include <imgui.h>

#include "Engine/Core/Log.h"
#include "Engine/GameFramework/Scene.h"
#include "Engine/Renderer/Buffer.h"
#include "Menus/EditorMenu.h"
#include "Panels/EditorPanel.h"
#include "Engine/Events/Event.h"

namespace ZeoEngine {

	class EditorDockspace
	{
		friend class DockspaceManager;

	protected:
		EditorDockspace() = delete;
		explicit EditorDockspace(const std::string& dockspaceName,
			float dockspaceRounding = 0.0f, float dockspaceBorderSize = 0.0f, ImVec2 dockspacePadding = ImVec2(0.0f, 0.0f),
			ImGuiWindowFlags dockspaceWindowFlags = ImGuiWindowFlags_MenuBar,
			ImVec2 dockspacePos = ImVec2(0.0f, 0.0f), ImVec2 dockspaceSize = ImVec2(700.0f, 500.0f));

	public:
		virtual void OnAttach();
		virtual void OnDetach() {}
		void OnUpdate(DeltaTime dt);
		void OnImGuiRender();
		void OnEvent(Event& e);

		const std::string& GetDockspaceName() const { return m_DockspaceName; }
		void SetShow(bool bShow) { m_bShow = bShow; }
		const Ref<Scene>& GetScene() const { return m_Scene; }
		const Ref<FrameBuffer>& GetFrameBuffer() const { return m_FBO; }

		void PushMenu(EditorMenu* menu);
		void PushPanel(EditorPanel* panel);
		EditorPanel* GetPanelByName(const std::string& panelName);

	private:
		void CreateScene();

		void CreateFrameBuffer();
		void BeginFrameBuffer();
		void EndFrameBuffer();

		void RenderDockspace();
		virtual void BuildDockWindows(ImGuiID dockspaceID) = 0;

	protected:
		bool m_bIsMainDockspace{ false };
	private:
		std::string m_DockspaceName;
		ImVec2 m_DockspacePos;
		ImVec2 m_DockspaceSize;
		float m_DockspaceRounding;
		float m_DockspaceBorderSize;
		ImVec2 m_DockspacePadding;
		ImGuiWindowFlags m_DockspaceWindowFlags;
		bool m_bShow{ true };

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

		/** If dockspaceName is not specified, all dockspaces will be reset layout. */
		void RebuildDockLayout(const std::string& dockspaceName = std::string());

	private:
		std::vector<EditorDockspace*> m_Dockspaces;
	};

}

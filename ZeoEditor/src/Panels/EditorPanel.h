#pragma once

#include <glm/glm.hpp>

#include "Engine/Core/Log.h"
#include "Engine/GameFramework/Scene.h"
#include "Engine/Renderer/Buffer.h"
#include "Engine/ImGui/MyImGui.h"
#include "Utils/EditorUtils.h"

namespace ZeoEngine {

	class EditorDockspace;

	class EditorPanel
	{
	public:
		EditorPanel() = delete;
		EditorPanel(EditorPanelType panelType, bool bDefaultShow = false, ImGuiWindowFlags panelWindowFlags = 0,
			const ImVec2Data& initialSize = ImVec2Data::DefaultSize, const ImVec2Data& initialPos = ImVec2Data::DefaultPos);
		virtual ~EditorPanel() = default;

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(DeltaTime dt) {}
		void OnImGuiRender();

		std::string GetPanelName() const { return std::move(ResolveEditorNameFromEnum(m_PanelType)); }
	protected:
		virtual bool IsShow() const { return m_bShow; }
	public:
		bool* GetShowPtr() { return &m_bShow; }
		/**
		  * In most cases, you should use ImGui::IsWindowHovered() instead of this inside ImGui::Begin()
		  * A known use case for this is inside SceneViewportPanel::SceneCameraController which is outside ImGui::Begin() context
		  */
		bool IsHovering() const { return m_bIsHovering; }

	private:
		virtual void RenderPanel() = 0;

	private:
		EditorPanelType m_PanelType;
		bool m_bShow;
		ImGuiWindowFlags m_PanelWindowFlags;
		ImVec2Data m_InitialPos, m_InitialSize;
		bool m_bIsHovering{ false };
	};

	class ScenePanel : public EditorPanel
	{
	public:
		ScenePanel() = delete;
		ScenePanel(EditorPanelType panelType, EditorDockspace* context, bool bDefaultShow = false, ImGuiWindowFlags panelWindowFlags = 0,
			const ImVec2Data& initialSize = ImVec2Data::DefaultSize, const ImVec2Data& initialPos = ImVec2Data::DefaultPos);

	protected:
		template<typename T=EditorDockspace>
		T* GetContext() { return dynamic_cast<T*>(m_Context); }
		virtual bool IsShow() const override;
		const Ref<Scene>& GetScene() const;
		const Ref<FrameBuffer>& GetFrameBuffer() const;

	private:
		EditorDockspace* m_Context;
	};

	class PanelManager
	{
	public:
		PanelManager() = default;
		~PanelManager();

		void OnUpdate(DeltaTime dt);
		void OnImGuiRender();

		void PushPanel(EditorPanel* panel);
		EditorPanel* GetPanelByName(const std::string& panelName);

	private:
		std::unordered_map<std::string, EditorPanel*> m_Panels;
	};

}

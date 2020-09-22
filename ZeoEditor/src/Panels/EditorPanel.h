#pragma once

#include <glm/glm.hpp>
#include <imgui/imgui.h>

#include "Engine/Core/Log.h"
#include "Engine/GameFramework/Scene.h"
#include "Engine/Renderer/Buffer.h"

namespace ZeoEngine {

	class EditorDockspace;

	struct Vec2Data
	{
		ImVec2 Data;
		ImGuiCond Condition{ ImGuiCond_FirstUseEver };

		static Vec2Data DefaultPos;
		static Vec2Data CenterPos;
		static Vec2Data DefaultSize;
	};

	class EditorPanel
	{
	public:
		EditorPanel() = delete;
		EditorPanel(const std::string& panelName, bool bDefaultShow = false, ImGuiWindowFlags panelWindowFlags = 0,
			const Vec2Data& initialSize = Vec2Data::DefaultSize, const Vec2Data& initialPos = Vec2Data::CenterPos);
		virtual ~EditorPanel() = default;

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnImGuiRender();

		const std::string& GetPanelName() const { return m_PanelName; }
		bool* GetShowPtr() { return &m_bShow; }
		void SetShow(bool bShow) { m_bShow = bShow; }
		bool IsHovering() const { return m_bIsHovering; }

	protected:
		std::string m_PanelName;
		bool m_bShow;
		ImGuiWindowFlags m_PanelWindowFlags;
		Vec2Data m_InitialPos, m_InitialSize;
		bool m_bIsHovering{ false };
	};

	class ScenePanel : public EditorPanel
	{
	public:
		ScenePanel(const std::string& panelName, EditorDockspace* context, bool bDefaultShow = false, ImGuiWindowFlags panelWindowFlags = 0,
			const Vec2Data& initialSize = Vec2Data::DefaultSize, const Vec2Data& initialPos = Vec2Data::CenterPos);

	protected:
		template<typename T>
		T* GetContext() { return dynamic_cast<T*>(m_Context); }
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

		void OnImGuiRender();

		void PushPanel(EditorPanel* panel);
		EditorPanel* GetPanelByName(const std::string& panelName);

	private:
		std::unordered_map<std::string, EditorPanel*> m_Panels;
	};

}

#pragma once

#include "Core/WindowManager.h"
#include "Engine/GameFramework/Scene.h"
#include "Engine/Renderer/Buffer.h"
#include "Engine/ImGui/MyImGui.h"
#include "Utils/EditorUtils.h"

namespace ZeoEngine {

	class EditorDockspace;

	class EditorPanel
	{
		friend class PanelManager;

	private:
		EditorPanel() = delete;
		EditorPanel(const EditorPanelSpec& spec, EditorDockspace* context);
	public:
		virtual ~EditorPanel() = default;

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(DeltaTime dt) {}
		void OnImGuiRender();
		virtual void OnEvent(Event& e) {}

	protected:
		const Ref<Scene>& GetScene() const;
		const Ref<FrameBuffer>& GetFrameBuffer() const;
	public:
		template<typename T = EditorDockspace>
		T* GetContext() { return dynamic_cast<T*>(m_Context); }
		bool* GetShowPtr() { return &m_bShow; }
		bool IsPanelFocused() const { return m_bIsPanelFocused; }
		bool IsPanelHovered() const { return m_bIsPanelHovered; }

	private:
		virtual void RenderPanel() = 0;

	protected:
		EditorPanelSpec m_PanelSpec;
	private:
		EditorDockspace* m_Context;
		bool m_bShow = true;
		bool m_bIsPanelFocused = false, m_bIsPanelHovered = false;
	};

}

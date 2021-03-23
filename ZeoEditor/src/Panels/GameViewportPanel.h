#pragma once

#include "Panels/SceneViewportPanel.h"

#include "Engine/Events/KeyEvent.h"
#include "Engine/Events/MouseEvent.h"
#include "Engine/GameFramework/Entity.h"

namespace ZeoEngine {

	class FrameBuffer;

	class GameViewportPanel : public SceneViewportPanel
	{
	public:
		using SceneViewportPanel::SceneViewportPanel;

		virtual void OnAttach() override;

	private:
		virtual void ProcessRender() override;

		virtual void ProcessEvent(Event& e) override;

		virtual void RenderToolbar() override;

		void RenderGizmo();

		void ProcessEntityDeletion();

		bool OnKeyPressed(KeyPressedEvent& e);
		bool OnMouseButtonPressed(MouseButtonPressedEvent& e);

		void ReadPixelDataFromIDBuffer(const Ref<FrameBuffer>& frameBuffer);

	private:
		int32_t m_GizmoType = 0;
		bool m_bGizmoVisible = false;

		Entity m_HoveredEntity;
	};

}

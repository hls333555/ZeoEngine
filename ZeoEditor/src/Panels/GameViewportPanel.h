#pragma once

#include "Panels/SceneViewportPanel.h"

#include "Engine/Events/KeyEvent.h"
#include "Engine/Events/MouseEvent.h"
#include "Engine/GameFramework/Entity.h"

namespace ZeoEngine {

	class GameViewportPanel : public SceneViewportPanel
	{
	public:
		using SceneViewportPanel::SceneViewportPanel;

		virtual void OnAttach() override;
		virtual void OnEvent(Event& e) override;

		void ReadPixelDataFromIDBuffer(const Ref<FrameBuffer>& frameBuffer);

	private:
		bool OnKeyPressed(KeyPressedEvent& e);
		bool OnMouseButtonPressed(MouseButtonPressedEvent& e);

		virtual void RenderPanel() override;

		virtual void RenderToolbar() override;

		void ToggleStopTexture();
		void ToggleResumeTexture();

		void RenderGizmo();

	private:
		Ref<Texture2D> m_StopTexture;
		
		int32_t m_GizmoType = 0;

		Entity m_HoveredEntity;
	};

}

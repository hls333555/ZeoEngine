#pragma once

#include "Panels/SceneViewportPanel.h"

namespace ZeoEngine {

	class GameViewportPanel : public SceneViewportPanel
	{
	public:
		using SceneViewportPanel::SceneViewportPanel;

		virtual void OnAttach() override;

	private:
		virtual void RenderPanel() override;

		virtual void RenderToolbar() override;

		void ToggleStopTexture();
		void ToggleResumeTexture();

		void RenderGizmo();

	private:
		Ref<Texture2D> m_StopTexture;
		
		int32_t m_GizmoType = -1;
	};

}

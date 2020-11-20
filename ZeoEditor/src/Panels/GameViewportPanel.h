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

	private:
		Ref<Texture2D> m_StopTexture;
		
	};

}

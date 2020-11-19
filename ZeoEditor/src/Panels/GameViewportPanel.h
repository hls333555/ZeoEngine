#pragma once

#include "Panels/SceneViewportPanel.h"

#include "Engine/Renderer/Texture.h"

namespace ZeoEngine {

	class GameViewportPanel : public SceneViewportPanel
	{
	public:
		using SceneViewportPanel::SceneViewportPanel;

		virtual void OnAttach() override;

	private:
		virtual void RenderPanel() override;
		virtual void RenderToolbar() override;

	private:
		Ref<Texture2D> m_PlayTexture, m_PauseTexture, m_StopTexture;
		void* m_ToolbarTextures[2];
	};

}

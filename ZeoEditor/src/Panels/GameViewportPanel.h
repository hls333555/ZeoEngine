#pragma once

#include "Panels/SceneViewportPanel.h"

#include "Engine/Renderer/Texture.h"

namespace ZeoEngine {

	class GameViewportPanel : public SceneViewportPanel
	{
	public:
		using SceneViewportPanel::SceneViewportPanel;

		virtual void OnAttach() override;
		virtual void OnImGuiRender() override;

	private:
		void RenderToolbar();

	private:
		Ref<Texture2D> m_PlayTexture, m_PauseTexture, m_StopTexture, m_ToolbarTextures[2];
	};

}

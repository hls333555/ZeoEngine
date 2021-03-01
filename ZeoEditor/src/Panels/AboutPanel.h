#pragma once

#include "Panels/EditorPanel.h"

#include "Engine/Renderer/Texture.h"

namespace ZeoEngine {

	class AboutPanel : public EditorPanel
	{
	public:
		using EditorPanel::EditorPanel;

		virtual void OnAttach() override;

	private:
		virtual void RenderPanel() override;

	private:
		Ref<Texture2D> m_LogoTexture;
	};

}

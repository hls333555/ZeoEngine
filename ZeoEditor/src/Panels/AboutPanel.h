#pragma once

#include "Panels/PanelBase.h"

#include "Engine/Renderer/Texture.h"

namespace ZeoEngine {

	class AboutPanel : public PanelBase
	{
	public:
		using PanelBase::PanelBase;

		virtual void OnAttach() override;

	private:
		virtual void ProcessRender() override;

	private:
		Ref<Texture2D> m_LogoTexture;
	};

}

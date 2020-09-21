#pragma once

#include "Panels/EditorPanel.h"

#include <glm/glm.hpp>

#include "Engine/GameFramework/Entity.h"

namespace ZeoEngine {

	class SceneViewportPanel : public ScenePanel
	{
	public:
		using ScenePanel::ScenePanel;
		virtual void OnAttach() override;
		virtual void OnImGuiRender() override;

	private:
		void CreatePreviewCamera();

		void OnViewportResize(const glm::vec2& size);

	private:
		Entity m_PreviewCamera;
		glm::vec2 m_LastViewportSize{ 0.0f };

	};

}

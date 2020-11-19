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

		virtual void OnUpdate(DeltaTime dt) override;

		void CreatePreviewCamera(bool bIsFromOpenScene = false);

		/**
		 * Snapshot current viewport and save it to a PNG file.
		 * If imageWidth is non-zero, it will snapshot a centered square area using this provided imageWidth, otherwise it will snapshot the full viewport.
		 * NOTE: Child classes should override this and update preview thumbnail if required.
		 */
		virtual void Snapshot(const std::string& imageName, uint32_t imageWidth = 0);

	protected:
		virtual void RenderPanel() override;
	private:
		virtual void RenderToolbar() {};

		void OnViewportResize(const glm::vec2& size);

		void InitCameraViewportSize(entt::registry& registry, entt::entity entity);

	private:
		Entity m_PreviewCamera;
		glm::vec2 m_LastViewportSize{ 0.0f };

	};

}

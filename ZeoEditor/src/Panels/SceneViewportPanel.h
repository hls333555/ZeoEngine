#pragma once

#include "Panels/EditorPanel.h"

#include <glm/glm.hpp>

#include "Engine/GameFramework/Entity.h"
#include "Engine/Renderer/Texture.h"
#include "Engine/Events/KeyEvent.h"
#include "Engine/Events/MouseEvent.h"

namespace ZeoEngine {

	class SceneViewportPanel : public ScenePanel
	{
	public:
		using ScenePanel::ScenePanel;
		virtual void OnAttach() override;
		virtual void OnUpdate(DeltaTime dt) override;
		virtual void OnEvent(Event& e) override;

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
		void UpdatePreviewCamera(DeltaTime dt);
		bool OnMouseScrolled(MouseScrolledEvent& e);
		bool OnKeyPressed(KeyPressedEvent& e);

		virtual void RenderToolbar() {};

		void OnViewportResize(const glm::vec2& size);

		void InitCameraViewportSize(entt::registry& registry, entt::entity entity);

	protected:
		Ref<Texture2D> m_PlayTexture, m_PauseTexture;
		void* m_ToolbarTextures[2];

		Entity m_PreviewCamera;

	private:
		glm::vec2 m_LastViewportSize{ 0.0f };

		bool m_bShouldUpdateCameraPan = false;
		glm::vec2 m_LastPressedMousePosition{ 0.0f };
		bool m_bIsMiddleMouseButtonFirstPressed = true, m_bIsMiddleMouseButtonFirstPressedWhenHovered = true;

	};

}

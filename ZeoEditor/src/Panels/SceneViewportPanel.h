#pragma once

#include "Panels/EditorPanel.h"

#include <glm/glm.hpp>

#include "Engine/Events/Event.h"
#include "Engine/Renderer/EditorCamera.h"

namespace ZeoEngine {

	class SceneViewportPanel : public EditorPanel
	{
	public:
		using EditorPanel::EditorPanel;
		virtual void OnAttach() override;
		virtual void OnUpdate(DeltaTime dt) override;
		virtual void OnEvent(Event& e) override;

		/**
		 * Snapshot current viewport and save it to a PNG file.
		 * If imageWidth is non-zero, it will snapshot a centered square area using this provided imageWidth, otherwise it will snapshot the full viewport.
		 * NOTE: Child classes should override this and update preview thumbnail if required.
		 */
		virtual void Snapshot(const std::string& imageName, uint32_t imageWidth = 0);

		const glm::vec2* GetViewportBounds() const { return m_ViewportBounds; }

		std::pair<float, float> GetMouseViewportPosition();

	protected:
		virtual void RenderPanel() override;

	private:
		virtual void RenderToolbar() {};

		void SetViewportBounds(float x, float y, float width, float height);

		void OnViewportResize(const glm::vec2& size);

	protected:
		EditorCamera m_EditorCamera;
		glm::vec2 m_ViewportBounds[2];

	private:
		glm::vec2 m_LastViewportSize{ 0.0f };

	};

}

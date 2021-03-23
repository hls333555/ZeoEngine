#pragma once

#include "Panels/PanelBase.h"

#include <glm/glm.hpp>

#include "Engine/Events/KeyEvent.h"
#include "Engine/Renderer/EditorCamera.h"

namespace ZeoEngine {

	class SceneViewportPanel : public PanelBase
	{
	public:
		using PanelBase::PanelBase;

		virtual void OnAttach() override;
		
		/**
		 * Snapshot current viewport and save it to a PNG file.
		 * If imageWidth is non-zero, it will snapshot a centered square area using this provided imageWidth, otherwise it will snapshot the full viewport.
		 * NOTE: Child classes should override this and update preview thumbnail if required.
		 */
		virtual void Snapshot(const std::string& imageName, uint32_t imageWidth = 0);

		const glm::vec2* GetViewportBounds() const { return m_ViewportBounds; }
		glm::vec2 GetViewportSize() const;

		std::pair<float, float> GetMouseViewportPosition();

	protected:
		virtual void ProcessRender() override;
		virtual void ProcessEvent(Event& e) override;

	private:
		virtual void ProcessUpdate(DeltaTime dt) override;

		virtual void RenderToolbar() {};

		bool OnMouseScroll(MouseScrolledEvent& e);
		bool OnFocusEntity(KeyPressedEvent& e);

		void SetViewportBounds(float x, float y, float width, float height);

		void OnViewportResize(const glm::vec2& size);

	protected:
		EditorCamera m_EditorCamera;
		glm::vec2 m_ViewportBounds[2];

	private:
		glm::vec2 m_LastViewportSize{ 0.0f };

	};

}

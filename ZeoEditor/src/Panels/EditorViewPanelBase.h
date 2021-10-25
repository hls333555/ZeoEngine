#pragma once

#include "Panels/PanelBase.h"

#include <glm/glm.hpp>

#include "Engine/Events/KeyEvent.h"
#include "Engine/Renderer/EditorCamera.h"

namespace ZeoEngine {

	class EditorViewPanelBase : public PanelBase
	{
	public:
		using PanelBase::PanelBase;

		virtual void OnAttach() override;
		
		/**
		 * Snapshot current viewport and save as thumbnail cache.
		 * 
		 * @param assetPath - Path of asset
		 * @param imageWidth - If non-zero, it will snapshot a centered square area using this provided imageWidth
		 * @param bOverwriteThumbnail - If true, it will always do a capture no matter the existance of local cache
		 */
		void Snapshot(const std::string& assetPath, uint32_t imageWidth = 0, bool bOverwriteThumbnail = true);

		const glm::vec2* GetViewportBounds() const { return m_ViewportBounds; }
		glm::vec2 GetViewportSize() const;

		std::pair<float, float> GetMouseViewportPosition();

	protected:
		virtual void ProcessRender() override;
		virtual void ProcessEvent(Event& e) override;

	private:
		virtual void ProcessUpdate(DeltaTime dt) override;

		virtual void RenderToolbar() {};

		virtual std::string GetPanelTitle() const override;

		bool OnMouseScroll(MouseScrolledEvent& e);
		bool OnKeyPressed(KeyPressedEvent& e);

		void OnViewportResize(const glm::vec2& size);

		void PostSceneCreate();

		void UpdateViewportSizeOnSceneCameras();

	protected:
		EditorCamera m_EditorCamera;
		glm::vec2 m_ViewportBounds[2];

	private:
		glm::vec2 m_LastViewportSize{ 0.0f };

		struct SnapshotSpec
		{
			std::string AssetPath;
			std::string ThumbnailPath;
			uint32_t ImageWidth;
		} m_SnapshotSpec;

	};

}

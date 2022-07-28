#pragma once

#include "Panels/PanelBase.h"

#include <glm/glm.hpp>

#include "Engine/Events/KeyEvent.h"
#include "Engine/Renderer/EditorCamera.h"

namespace ZeoEngine {

	struct AssetMetadata;

	class EditorViewPanelBase : public PanelBase
	{
	public:
		using PanelBase::PanelBase;

		virtual void OnAttach() override;
		
		/**
		 * Snapshot current viewport and save as thumbnail cache.
		 * 
		 * @param metadata - Metadata of asset
		 * @param imageWidth - If non-zero, it will snapshot a centered square area using this provided imageWidth
		 * @param bOverwriteThumbnail - If true, it will always do a capture no matter the existance of local cache
		 */
		void Snapshot(const Ref<AssetMetadata>& metadata, U32 imageWidth = 0, bool bOverwriteThumbnail = true) const;

		const Vec2* GetViewportBounds() const { return m_ViewportBounds; }
		Vec2 GetViewportSize() const;

		std::pair<float, float> GetMouseViewportPosition() const;

	protected:
		virtual void ProcessRender() override;
		virtual void ProcessEvent(Event& e) override;

	private:
		virtual void ProcessUpdate(DeltaTime dt) override;

		virtual void RenderToolbar() {};

		virtual std::string GetPanelTitle() const override;

		bool OnMouseScroll(MouseScrolledEvent& e);
		bool OnKeyPressed(KeyPressedEvent& e);

		void OnViewportResize(const Vec2& size);

		void PostSceneCreate(const Ref<Scene>& scene);

		void UpdateViewportSizeOnSceneCameras();

	protected:
		EditorCamera m_EditorCamera;
		Vec2 m_ViewportBounds[2];

	private:
		Vec2 m_LastViewportSize{ 0.0f };

	};

}

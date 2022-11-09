#pragma once

#include <entt.hpp>

#include "Panels/PanelBase.h"

#include "Engine/Events/KeyEvent.h"
#include "Engine/Events/MouseEvent.h"

namespace ZeoEngine {

	class EditorPreviewWorldBase;
	class FrameBuffer;
	struct CameraComponent;
	class Scene;

	class ViewPanelBase : public PanelBase
	{
	public:
		using PanelBase::PanelBase;

		void UpdateWorld(EditorPreviewWorldBase* world);
		
		/**
		 * Snapshot current viewport and save as thumbnail cache.
		 * 
		 * @param imageWidth - If non-zero, it will snapshot a centered square area using this provided imageWidth
		 * @param bOverwriteThumbnail - If true, it will always do a capture no matter the existance of local cache
		 */
		void Snapshot(U32 imageWidth = 0, bool bOverwriteThumbnail = true) const;

		const Vec2* GetViewportBounds() const { return m_ViewportBounds; }
		Vec2 GetViewportSize() const;

		std::pair<float, float> GetMouseViewportPosition() const;

		void OnCameraComponentAdded(Scene& scene, entt::entity e) const;

	protected:
		virtual void ProcessRender() override;
		virtual void ProcessEvent(Event& e) override;

		virtual void OnWorldChanged(EditorPreviewWorldBase* world, EditorPreviewWorldBase* lastWorld);

		EditorPreviewWorldBase* GetEditorWorld() const { return m_EditorWorld; }

	private:
		virtual void ProcessUpdate(DeltaTime dt) override;

		virtual std::string GetPanelTitle() const override;

		bool OnMouseScroll(MouseScrolledEvent& e) const;
		bool OnKeyPressed(KeyPressedEvent& e) const;

		void OnViewportResize(const Vec2& size) const;

		/** Resize non-FixedAspectRatio camera. */
		void UpdateViewportSizeOnSceneCamera(CameraComponent& cameraComp) const;

	public:
		entt::sink<entt::sigh<void(U32, U32)>> m_OnViewportResize{ m_OnViewportResizeDel };

	private:
		EditorPreviewWorldBase* m_EditorWorld = nullptr;
		Ref<FrameBuffer> m_FrameBuffer;

		Vec2 m_ViewportBounds[2];
		Vec2 m_LastViewportSize{ 0.0f };
		entt::sigh<void(U32, U32)> m_OnViewportResizeDel;

	};

}

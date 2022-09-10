#pragma once

#include <entt.hpp>

#include "Panels/PanelBase.h"

#include "Engine/Events/KeyEvent.h"
#include "Engine/Events/MouseEvent.h"

namespace ZeoEngine {

	struct AssetMetadata;
	class Scene;
	class EditorPreviewWorldBase;
	class FrameBuffer;

	class ViewPanelBase : public PanelBase
	{
	public:
		using PanelBase::PanelBase;

		void UpdateWorld(const Ref<EditorPreviewWorldBase>& world);
		
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

	protected:
		virtual void ProcessRender() override;
		virtual void ProcessEvent(Event& e) override;

		EditorPreviewWorldBase* GetEditorWorld() const { return m_EditorWorld; }

		void UpdateViewportSizeOnSceneCameras() const;

	private:
		virtual void ProcessUpdate(DeltaTime dt) override;

		virtual std::string GetPanelTitle() const override;

		bool OnMouseScroll(MouseScrolledEvent& e) const;
		bool OnKeyPressed(KeyPressedEvent& e) const;

		void OnViewportResize(const Vec2& size) const;

		void BindCameraComponentConstructionDelegate(const Ref<Scene>& scene);
		void UnbindCameraComponentConstructionDelegate(const Ref<Scene>& scene);

	public:
		entt::sink<entt::sigh<void(U32, U32)>> m_OnViewportResize{ m_OnViewportResizeDel };

	private:
		EditorPreviewWorldBase* m_EditorWorld = nullptr;
		FrameBuffer* m_FrameBuffer = nullptr;

		Vec2 m_ViewportBounds[2];
		Vec2 m_LastViewportSize{ 0.0f };
		entt::sigh<void(U32, U32)> m_OnViewportResizeDel;

	};

}

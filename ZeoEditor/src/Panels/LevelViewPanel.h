#pragma once

#include "Panels/ViewPanelBase.h"

#include <ImGuizmo.h>

#include "Engine/Events/KeyEvent.h"
#include "Engine/Events/MouseEvent.h"
#include "Engine/GameFramework/Entity.h"

namespace ZeoEngine {

	class FrameBuffer;
	class LevelPreviewWorld;

	class LevelViewPanel : public ViewPanelBase
	{
	public:
		explicit LevelViewPanel(std::string panelName);

		virtual void OnAttach() override;

	private:
		virtual void ProcessRender() override;
		virtual void ProcessEvent(Event& e) override;

		virtual void OnWorldChanged(const Ref<EditorPreviewWorldBase>& world, const Ref<EditorPreviewWorldBase>& lastWorld) override;
		void OnActiveSceneChanged(const Ref<Scene>& scene, const Ref<Scene>& lastScene);

		void BindCameraComponentConstructionDelegate(const Ref<Scene>& scene);

		void RenderToolbar(const Ref<LevelPreviewWorld>& levelWorld) const;
		void RenderGizmo(const Ref<LevelPreviewWorld>& levelWorld);

		bool OnKeyPressed(KeyPressedEvent& e);
		bool OnMouseButtonPressed(MouseButtonPressedEvent& e) const;

		void ReadPixelDataFromIDBuffer(const Ref<FrameBuffer>& frameBuffer);

	private:
		Weak<LevelPreviewWorld> m_LevelWorld;

		ImGuizmo::OPERATION m_GizmoType = ImGuizmo::TRANSLATE;
		bool m_bGizmoVisible = false;

		Entity m_HoveredEntity;
	};

}

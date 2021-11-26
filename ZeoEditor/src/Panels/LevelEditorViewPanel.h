#pragma once

#include "Panels/EditorViewPanelBase.h"

#include <ImGuizmo.h>

#include "Engine/Events/KeyEvent.h"
#include "Engine/Events/MouseEvent.h"
#include "Engine/GameFramework/Entity.h"

namespace ZeoEngine {

	class FrameBuffer;

	class LevelEditorViewPanel : public EditorViewPanelBase
	{
	public:
		using EditorViewPanelBase::EditorViewPanelBase;

		virtual void OnAttach() override;

	private:
		virtual void ProcessRender() override;
		virtual void ProcessEvent(Event& e) override;

		void PostSceneRender(const Ref<FrameBuffer>& frameBuffer);

		virtual void RenderToolbar() override;

		void RenderGizmo();

		bool OnKeyPressed(KeyPressedEvent& e);
		bool OnMouseButtonPressed(MouseButtonPressedEvent& e);

		void ReadPixelDataFromIDBuffer(const Ref<FrameBuffer>& frameBuffer);

	private:
		ImGuizmo::OPERATION m_GizmoType = ImGuizmo::TRANSLATE;
		bool m_bGizmoVisible = false;

		Entity m_HoveredEntity;
	};

}

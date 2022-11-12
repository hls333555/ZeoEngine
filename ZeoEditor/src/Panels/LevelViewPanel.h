#pragma once

#include "Panels/ViewPanelBase.h"

#include <ImGuizmo.h>

#include "Engine/Events/KeyEvent.h"
#include "Engine/Events/MouseEvent.h"
#include "Engine/GameFramework/Entity.h"

namespace ZeoEngine {

	class FrameBuffer;

	class LevelViewPanel : public ViewPanelBase
	{
	public:
		explicit LevelViewPanel(std::string panelName);

		virtual void OnAttach() override;

	private:
		virtual void ProcessRender() override;
		virtual void ProcessEvent(Event& e) override;

		void RenderToolbar();
		void RenderGizmo();

		void SetTranslationGizmo();
		void SetRotationGizmo();
		void SetScaleGizmo();
		void ToggleGizmoType();
		void ToggleGizmoMode();

		bool OnKeyPressed(KeyPressedEvent& e);
		bool OnMouseButtonPressed(MouseButtonPressedEvent& e) const;

		void ReadPixelDataFromIDBuffer(FrameBuffer& frameBuffer);

	private:
		ImGuizmo::OPERATION m_GizmoType = ImGuizmo::TRANSLATE;
		ImGuizmo::MODE m_GizmoMode = ImGuizmo::WORLD;
		bool m_bGizmoVisible = false;

		Entity m_HoveredEntity;
	};

}

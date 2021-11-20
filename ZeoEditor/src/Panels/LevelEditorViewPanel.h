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

		void RenderGrid();

	private:
		ImGuizmo::OPERATION m_GizmoType = ImGuizmo::TRANSLATE;
		bool m_bGizmoVisible = false;

		struct GridData
		{
			glm::mat4 Transform = glm::mat4(1.0f);
			glm::vec4 ThinLinesColor{ 0.2f, 0.2f, 0.2f, 0.3f };
			glm::vec4 ThickLinesColor{ 0.5f, 0.5f, 0.5f, 0.3f };
			glm::vec4 OriginAxisXColor{ 1.0f, 0.0f, 0.0f, 0.3f };
			glm::vec4 OriginAxisZColor{ 0.0f, 0.0f, 1.0f, 0.3f };
			float Extent = 101.0f;
			float CellSize = 0.025f;
			int32_t InstanceCount = 10;
		};
		GridData m_GridBuffer;
		Ref<UniformBuffer> m_GridUniformBuffer;
		Ref<Shader> m_GridShader;

		Entity m_HoveredEntity;
	};

}

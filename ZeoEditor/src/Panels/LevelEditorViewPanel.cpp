#include "Panels/LevelEditorViewPanel.h"

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>
#include <IconsFontAwesome5.h>

#include "Engine/GameFramework/Components.h"
#include "Engine/Math/Math.h"
#include "Engine/Core/Input.h"
#include "Engine/Core/KeyCodes.h"
#include "Engine/Core/MouseCodes.h"
#include "Engine/Renderer/Renderer2D.h"
#include "Editors/EditorBase.h"
#include "Editors/LevelEditor.h"

namespace ZeoEngine {

	void LevelEditorViewPanel::OnAttach()
	{
		EditorViewPanelBase::OnAttach();

		GetContextEditor()->m_PostSceneRender.connect<&LevelEditorViewPanel::PostSceneRender>(this);
	}

	void LevelEditorViewPanel::ProcessRender()
	{
		EditorViewPanelBase::ProcessRender();

		RenderGizmo();

	}

	void LevelEditorViewPanel::ProcessEvent(Event& e)
	{
		EditorViewPanelBase::ProcessEvent(e);

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>(ZE_BIND_EVENT_FUNC(LevelEditorViewPanel::OnKeyPressed));
		dispatcher.Dispatch<MouseButtonPressedEvent>(ZE_BIND_EVENT_FUNC(LevelEditorViewPanel::OnMouseButtonPressed));
	}

	void LevelEditorViewPanel::PostSceneRender(const Ref<FrameBuffer>& frameBuffer)
	{
		ReadPixelDataFromIDBuffer(frameBuffer);
	}

	void LevelEditorViewPanel::RenderToolbar()
	{
		auto sceneEditor = GetContextEditor<LevelEditor>();
		// Place buttons at window center
		float indent = sceneEditor->GetSceneState() > SceneState::Edit ? ImGui::GetContentRegionAvail().x * 0.5f - ImGui::GetFrameHeightWithSpacing() :
			(ImGui::GetContentRegionAvail().x - ImGui::GetFontSize()) * 0.5f - ImGui::GetFramePadding().x;
		ImGui::Indent(indent);

		// Toggle play / stop
		if (ImGui::TransparentButton(sceneEditor->GetSceneState() > SceneState::Edit ? ICON_FA_STOP : ICON_FA_PLAY))
		{
			switch (sceneEditor->GetSceneState())
			{
				case SceneState::Edit:
					sceneEditor->OnScenePlay();
					break;
				case SceneState::Play:
				case SceneState::Pause:
					sceneEditor->OnSceneStop();
					break;
			}
		}

		// Toggle pause / resume
		switch (sceneEditor->GetSceneState())
		{
			case SceneState::Play:
				ImGui::SameLine();
				if (ImGui::TransparentButton(ICON_FA_PAUSE))
				{
					sceneEditor->OnScenePause();
				}
				break;
			case SceneState::Pause:
				ImGui::SameLine();
				if (ImGui::TransparentButton(ICON_FA_PLAY))
				{
					sceneEditor->OnSceneResume();
				}
				break;
		}
	}

	void LevelEditorViewPanel::RenderGizmo()
	{
		Entity selectedEntity = GetContextEditor()->GetContextEntity();
		m_bGizmoVisible = selectedEntity && m_GizmoType != -1;
		if (m_bGizmoVisible)
		{
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();
			const glm::vec2* viewportBounds = GetViewportBounds();
			const glm::vec2 viewportSize = GetViewportSize();
			ImGuizmo::SetRect(viewportBounds[0].x, viewportBounds[0].y, viewportSize.x, viewportSize.y);

			// Editor camera
			const glm::mat4& cameraProjection = m_EditorCamera.GetProjection();
			glm::mat4 cameraView = m_EditorCamera.GetViewMatrix();

			// Selected entity
			auto& transformComp = selectedEntity.GetComponent<TransformComponent>();
			glm::mat4 entityTransform = transformComp.GetTransform();

			// Snapping
			bool bSnap = Input::IsKeyPressed(Key::LeftControl);
			// Snap to 0.5m for translation/scale
			float snapValue = 0.5f;
			// Snap to 45 degrees for rotation
			if (m_GizmoType == ImGuizmo::ROTATE)
			{
				snapValue = 45.0f;
			}
			float snapValues[3] = { snapValue, snapValue, snapValue };

			ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
				m_GizmoType, ImGuizmo::WORLD,
				glm::value_ptr(entityTransform),
				nullptr, bSnap ? snapValues : nullptr);

			if (ImGuizmo::IsUsing())
			{
				glm::vec3 outTranslation, outRotation, outScale;
				Math::DecomposeTransform(entityTransform, outTranslation, outRotation, outScale);

				// This delta rotation prevents gimbal lock situation
				glm::vec3 deltaRotation = outRotation - transformComp.Rotation;
				transformComp.Translation = outTranslation;
				transformComp.Rotation += deltaRotation;
				transformComp.Scale = outScale;
			}
		}
	}

	bool LevelEditorViewPanel::OnKeyPressed(KeyPressedEvent& e)
	{
		bool bIsCtrlPressed = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		bool bIsAltPressed = Input::IsKeyPressed(Key::LeftAlt) || Input::IsKeyPressed(Key::RightAlt);

		// Global responsing shotcuts
		{
			auto sceneEditor = GetContextEditor<LevelEditor>();
			switch (e.GetKeyCode())
			{
				// Toggle play
				case Key::P:
				{
					if (bIsAltPressed)
					{
						if (sceneEditor->GetSceneState() == SceneState::Edit)
						{
							sceneEditor->OnScenePlay();
							return true;
						}
					}
					break;
				}
				// Toggle pause / resume
				case Key::Pause:
				{
					switch (sceneEditor->GetSceneState())
					{
						case SceneState::Play:	sceneEditor->OnScenePause();	return true;
						case SceneState::Pause:	sceneEditor->OnSceneResume();	return true;
					}
				}
				// Toggle stop
				case Key::Escape:
				{
					if (sceneEditor->GetSceneState() > SceneState::Edit)
					{
						sceneEditor->OnSceneStop();
						return true;
					}
					break;
				}
			}
		}

		if ((!IsPanelFocused() && !IsPanelHovered()) || e.GetRepeatCount() > 0) return false;

		// Panel responsing shotcuts
		{
			bool bCanSwitchGizmo = !ImGuizmo::IsUsing();
			switch (e.GetKeyCode())
			{
				case Key::W:
				{
					if (bCanSwitchGizmo) m_GizmoType = ImGuizmo::TRANSLATE; return true;
				}
				case Key::E:
				{
					if (bCanSwitchGizmo) m_GizmoType = ImGuizmo::ROTATE; return true;
				}
				case Key::R:
				{
					if (bCanSwitchGizmo) m_GizmoType = ImGuizmo::SCALE; return true;
				}
				case Key::Space:
				{
					if (bCanSwitchGizmo)
					{
						switch (m_GizmoType)
						{
							case ImGuizmo::TRANSLATE:	m_GizmoType = ImGuizmo::ROTATE;		break;
							case ImGuizmo::ROTATE:		m_GizmoType = ImGuizmo::SCALE;		break;
							case ImGuizmo::SCALE:		m_GizmoType = ImGuizmo::TRANSLATE;	break;
						}
					}
					return true;
				}
				case Key::Delete:
				{
					GetContextEditor<LevelEditor>()->OnDeleteEntity();
					return true;
				}
				case Key::D:
				{
					if (bIsCtrlPressed)
					{
						GetContextEditor<LevelEditor>()->OnDuplicateEntity();
						return true;
					}
					break;
				}
			}
		}

		return false;
	}

	bool LevelEditorViewPanel::OnMouseButtonPressed(MouseButtonPressedEvent& e)
	{
		if (IsPanelHovered() &&
			e.GetMouseButton() == Mouse::ButtonLeft && !Input::IsKeyPressed(Key::CameraControl) &&
			(!m_bGizmoVisible || (m_bGizmoVisible && !ImGuizmo::IsOver())))
		{
			GetContextEditor()->SetContextEntity(m_HoveredEntity);
		}

		return false;
	}

	void LevelEditorViewPanel::ReadPixelDataFromIDBuffer(const Ref<FrameBuffer>& frameBuffer)
	{
		auto [mx, my] = GetMouseViewportPosition();
		glm::vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];
		// Y coordinate for framebuffer texture is from bottom to up
		my = viewportSize.y - my;
		if (mx >= 0.0f && my >= 0.0f && mx < viewportSize.x && my < viewportSize.y)
		{
			glm::vec4 pixel;
			frameBuffer->ReadPixel(1, static_cast<int32_t>(mx), static_cast<int32_t>(my), glm::value_ptr(pixel));
			m_HoveredEntity = pixel.x == -1 ? Entity{} : Entity(static_cast<entt::entity>(pixel.x), GetContextEditor()->GetScene().get());

			auto& Stats = Renderer::GetStats();
			Stats.HoveredEntity = m_HoveredEntity;
		}
	}

}

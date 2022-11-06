#include "Panels/LevelViewPanel.h"

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>
#include <IconsFontAwesome5.h>

#include "Core/Editor.h"
#include "Engine/GameFramework/Components.h"
#include "Engine/Math/Math.h"
#include "Engine/Core/Input.h"
#include "Engine/Core/KeyCodes.h"
#include "Engine/Core/MouseCodes.h"
#include "Engine/Renderer/Renderer2D.h"
#include "Engine/Core/Application.h"
#include "Worlds/LevelPreviewWorld.h"
#include "Engine/Renderer/SceneRenderer.h"

namespace ZeoEngine {

	LevelViewPanel::LevelViewPanel(std::string panelName)
		: ViewPanelBase(std::move(panelName))
	{
		UpdateWorld(g_Editor->GetLevelWorld());
	}

	void LevelViewPanel::OnAttach()
	{
		ViewPanelBase::OnAttach();

		m_LevelWorld = dynamic_cast<LevelPreviewWorld*>(GetEditorWorld());
		GetEditorWorld()->GetSceneRenderer()->m_PostSceneRender.connect<&LevelViewPanel::ReadPixelDataFromIDBuffer>(this);
	}

	void LevelViewPanel::ProcessRender()
	{
		const auto startPos = ImGui::GetCursorPos();

		ViewPanelBase::ProcessRender();

		RenderGizmo();

		ImGui::SetCursorPos(startPos);
		RenderToolbar();
	}

	void LevelViewPanel::ProcessEvent(Event& e)
	{
		ViewPanelBase::ProcessEvent(e);

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>(ZE_BIND_EVENT_FUNC(LevelViewPanel::OnKeyPressed));
		dispatcher.Dispatch<MouseButtonPressedEvent>(ZE_BIND_EVENT_FUNC(LevelViewPanel::OnMouseButtonPressed));
	}

	void LevelViewPanel::RenderToolbar() const
	{
		// Place buttons at window center
		float indent = m_LevelWorld->GetSceneState() > SceneState::Edit ? ImGui::GetContentRegionAvail().x * 0.5f - ImGui::GetFrameHeightWithSpacing() :
			(ImGui::GetContentRegionAvail().x - ImGui::GetFontSize()) * 0.5f - ImGui::GetFramePadding().x;
		ImGui::Indent(indent);

		// Toggle play / stop
		if (ImGui::TransparentButton(m_LevelWorld->GetSceneState() > SceneState::Edit ? ICON_FA_STOP : ICON_FA_PLAY))
		{
			if (m_LevelWorld->IsRuntime())
			{
				m_LevelWorld->OnSceneStop();
			}
			else
			{
				m_LevelWorld->OnScenePlay();
			}
		}

		ImGui::SameLine();

		// Toggle pause / resume
		switch (m_LevelWorld->GetSceneState())
		{
			case SceneState::Play:
				if (ImGui::TransparentButton(ICON_FA_PAUSE))
				{
					m_LevelWorld->OnScenePause();
				}
				break;
			case SceneState::Pause:
				if (ImGui::TransparentButton(ICON_FA_PLAY))
				{
					m_LevelWorld->OnSceneResume();
				}

				ImGui::SameLine();
				if (ImGui::TransparentButton(ICON_FA_STEP_FORWARD))
				{
					m_LevelWorld->OnSceneStep();
				}
				break;
		}
	}

	void LevelViewPanel::RenderGizmo()
	{
		Entity selectedEntity = m_LevelWorld->GetContextEntity();
		m_bGizmoVisible = selectedEntity && m_GizmoType != -1;
		if (m_bGizmoVisible)
		{
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();
			const Vec2* viewportBounds = GetViewportBounds();
			const Vec2 viewportSize = GetViewportSize();
			ImGuizmo::SetRect(viewportBounds[0].x, viewportBounds[0].y, viewportSize.x, viewportSize.y);

			// Editor camera
			const auto& editorCamera = m_LevelWorld->GetEditorCamera();
			const Mat4& cameraProjection = editorCamera.GetProjection();
			Mat4 cameraView = editorCamera.GetViewMatrix();

			// Selected entity
			auto& transformComp = selectedEntity.GetComponent<TransformComponent>();
			Mat4 entityTransform = transformComp.GetTransform();

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
				m_GizmoType, ImGuizmo::LOCAL,
				glm::value_ptr(entityTransform),
				nullptr, bSnap ? snapValues : nullptr);

			if (ImGuizmo::IsUsing())
			{
				Vec3 outTranslation, outRotation, outScale;
				Math::DecomposeTransform(entityTransform, outTranslation, outRotation, outScale);

				// This delta rotation prevents gimbal lock situation
				Vec3 deltaRotation = outRotation - transformComp.GetRotationInRadians();
				selectedEntity.SetTransform(outTranslation, transformComp.GetRotationInRadians() + deltaRotation, outScale);
			}
		}
	}

	bool LevelViewPanel::OnKeyPressed(KeyPressedEvent& e)
	{
		bool bIsCtrlPressed = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		bool bIsAltPressed = Input::IsKeyPressed(Key::LeftAlt) || Input::IsKeyPressed(Key::RightAlt);

		// Global shortcuts
		{
			switch (e.GetKeyCode())
			{
				// Toggle play
				case Key::P:
				{
					if (bIsAltPressed)
					{
						if (!m_LevelWorld->IsRuntime())
						{
							m_LevelWorld->OnScenePlay();
							return true;
						}
					}
					break;
				}
				// Step a frame
				case Key::F10:
				{
					if (m_LevelWorld->GetSceneState() == SceneState::Pause)
					{
						m_LevelWorld->OnSceneStep();
						return true;
					}
					break;
				}
				// Toggle pause / resume
				case Key::Pause:
				{
					switch (m_LevelWorld->GetSceneState())
					{
						case SceneState::Play:	m_LevelWorld->OnScenePause();	return true;
						case SceneState::Pause:	m_LevelWorld->OnSceneResume(); return true;
					}
				}
				// Toggle stop
				case Key::Escape:
				{
					if (m_LevelWorld->IsRuntime())
					{
						m_LevelWorld->OnSceneStop();
						return true;
					}
					break;
				}
				case Key::F9:
				{
					// TODO:
					Application::Get().GetRenderDoc().ToggleEnableCapture();
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
					m_LevelWorld->OnDeleteEntity();
					return true;
				}
				case Key::D:
				{
					if (bIsCtrlPressed)
					{
						m_LevelWorld->OnDuplicateEntity();
						return true;
					}
					break;
				}
			}
		}

		return false;
	}

	bool LevelViewPanel::OnMouseButtonPressed(MouseButtonPressedEvent& e) const
	{
		if (IsPanelHovered() &&
			e.GetMouseButton() == Mouse::ButtonLeft && !Input::IsKeyPressed(Key::CameraControl) &&
			(!m_bGizmoVisible || (m_bGizmoVisible && !ImGuizmo::IsOver())))
		{
			GetEditorWorld()->SetContextEntity(m_HoveredEntity);
		}

		return false;
	}

	void LevelViewPanel::ReadPixelDataFromIDBuffer(FrameBuffer& frameBuffer)
	{
		auto [mx, my] = GetMouseViewportPosition();
		Vec2 viewportSize = GetViewportBounds()[1] - GetViewportBounds()[0];
		// Y coordinate for framebuffer texture is from bottom to up
		my = viewportSize.y - my;
		if (mx >= 0.0f && my >= 0.0f && mx < viewportSize.x && my < viewportSize.y)
		{
			Vec4 pixel;
			frameBuffer.ReadPixel(1, static_cast<I32>(mx), static_cast<I32>(my), glm::value_ptr(pixel));
			m_HoveredEntity = pixel.x == -1 ? Entity{} : Entity{ static_cast<entt::entity>(pixel.x), GetEditorWorld()->GetActiveScene() };

			auto& Stats = Renderer::GetStats();
			Stats.HoveredEntity = m_HoveredEntity;
		}
	}

}

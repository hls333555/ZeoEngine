#include "Panels/GameViewportPanel.h"

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
#include "Editors/MainEditor.h"

namespace ZeoEngine {

	void GameViewportPanel::OnAttach()
	{
		SceneViewportPanel::OnAttach();

		GetOwningEditor()->m_PostSceneRender.connect<&GameViewportPanel::ReadPixelDataFromIDBuffer>(this);
	}

	void GameViewportPanel::ProcessRender()
	{
		SceneViewportPanel::ProcessRender();

		RenderGizmo();

		ProcessEntityDeletion();

	}

	void GameViewportPanel::ProcessEvent(Event& e)
	{
		SceneViewportPanel::ProcessEvent(e);

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>(ZE_BIND_EVENT_FUNC(GameViewportPanel::OnKeyPressed));
		dispatcher.Dispatch<MouseButtonPressedEvent>(ZE_BIND_EVENT_FUNC(GameViewportPanel::OnMouseButtonPressed));
	}

	void GameViewportPanel::RenderToolbar()
	{
		auto* mainEditor = GetOwningEditor<MainEditor>();
		// Place buttons at window center
		float indent = mainEditor->GetSceneState() > SceneState::Edit ? ImGui::GetContentRegionAvail().x * 0.5f - ImGui::GetFrameHeightWithSpacing() :
			(ImGui::GetContentRegionAvail().x - ImGui::GetFontSize()) * 0.5f - ImGui::GetFramePadding().x;
		ImGui::Indent(indent);

		// Toggle play / stop
		if (ImGui::TransparentButton(mainEditor->GetSceneState() > SceneState::Edit ? ICON_FA_STOP : ICON_FA_PLAY))
		{
			switch (mainEditor->GetSceneState())
			{
				case SceneState::Edit:
					mainEditor->OnScenePlay();
					break;
				case SceneState::Play:
				case SceneState::Pause:
					mainEditor->OnSceneStop();
					break;
			}
		}

		// Toggle pause / resume
		switch (mainEditor->GetSceneState())
		{
			case SceneState::Play:
				ImGui::SameLine();
				if (ImGui::TransparentButton(ICON_FA_PAUSE))
				{
					mainEditor->OnScenePause();
				}
				break;
			case SceneState::Pause:
				ImGui::SameLine();
				if (ImGui::TransparentButton(ICON_FA_PLAY))
				{
					mainEditor->OnSceneResume();
				}
				break;
		}
	}

	void GameViewportPanel::RenderGizmo()
	{
		ImGuizmo::Enable(!m_EditorCamera.IsUsing());

		Entity selectedEntity = GetOwningEditor()->GetContextEntity();
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
				m_GizmoType, ImGuizmo::LOCAL,
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

	void GameViewportPanel::ProcessEntityDeletion()
	{
		EditorBase* owningEditor = GetOwningEditor();
		Entity selectedEntity = owningEditor->GetContextEntity();
		if (IsPanelFocused() && selectedEntity && ImGui::IsKeyReleased(Key::Delete))
		{
			owningEditor->GetScene()->DestroyEntity(selectedEntity);
			owningEditor->SetContextEntity({});
		}
	}

	bool GameViewportPanel::OnKeyPressed(KeyPressedEvent& e)
	{
		// Global shotcuts
		{
			auto* mainEditor = GetOwningEditor<MainEditor>();
			// Toggle play
			if ((Input::IsKeyPressed(Key::LeftAlt) || Input::IsKeyPressed(Key::RightAlt)) && Input::IsKeyPressed(Key::P))
			{
				if (mainEditor->GetSceneState() == SceneState::Edit)
				{
					mainEditor->OnScenePlay();
				}
			}

			// Toggle pause / resume
			if (Input::IsKeyPressed(Key::Pause))
			{
				switch (mainEditor->GetSceneState())
				{
				case SceneState::Play:
					mainEditor->OnScenePause();
					break;
				case SceneState::Pause:
					mainEditor->OnSceneResume();
					break;
				}
			}

			// Toggle stop
			if (Input::IsKeyPressed(Key::Escape))
			{
				if (mainEditor->GetSceneState() > SceneState::Edit)
				{
					mainEditor->OnSceneStop();
				}
			}
		}

		if ((!IsPanelFocused() && !IsPanelHovered()) || e.GetRepeatCount() > 0) return false;

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
						case ImGuizmo::TRANSLATE:	m_GizmoType = ImGuizmo::ROTATE; break;
						case ImGuizmo::ROTATE:		m_GizmoType = ImGuizmo::SCALE; break;
						case ImGuizmo::SCALE:		m_GizmoType = ImGuizmo::TRANSLATE; break;
					}
				}
				return true;
			}
		}

		return false;
	}

	bool GameViewportPanel::OnMouseButtonPressed(MouseButtonPressedEvent& e)
	{
		if (IsPanelHovered() &&
			e.GetMouseButton() == Mouse::ButtonLeft && !Input::IsKeyPressed(Key::CameraControl) &&
			(!m_bGizmoVisible || (m_bGizmoVisible && !ImGuizmo::IsOver())))
		{
			GetOwningEditor()->SetContextEntity(m_HoveredEntity);
		}

		return false;
	}

	void GameViewportPanel::ReadPixelDataFromIDBuffer(const Ref<FrameBuffer>& frameBuffer)
	{
		auto [mx, my] = GetMouseViewportPosition();
		glm::vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];
		// Y coordinate for framebuffer texture is from bottom to up
		my = viewportSize.y - my;
		if (mx >= 0.0f && my >= 0.0f && mx < viewportSize.x && my < viewportSize.y)
		{
			int32_t pixel = frameBuffer->ReadPixel(1, static_cast<int32_t>(mx), static_cast<int32_t>(my));
			m_HoveredEntity = pixel == -1 ? Entity{} : Entity(static_cast<entt::entity>(pixel), GetOwningEditor()->GetScene().get());

			auto& Stats = Renderer2D::GetStats();
			Stats.HoveredEntity = m_HoveredEntity;
		}
	}

}

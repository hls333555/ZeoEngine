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

		// Call these manually as delegates are bound after the initial scene creation
		BindCameraComponentConstructionDelegate(GetEditorWorld()->GetActiveScene());
		UpdateViewportSizeOnSceneCameras();
	}

	void LevelViewPanel::OnAttach()
	{
		ViewPanelBase::OnAttach();

		m_LevelWorld = std::dynamic_pointer_cast<LevelPreviewWorld>(GetEditorWorld());
		GetEditorWorld()->GetSceneRenderer()->m_PostSceneRender.connect<&LevelViewPanel::ReadPixelDataFromIDBuffer>(this);
	}

	void LevelViewPanel::ProcessRender()
	{
		const auto startPos = ImGui::GetCursorPos();

		ViewPanelBase::ProcessRender();

		const auto levelWorld = m_LevelWorld.lock();
		RenderGizmo(levelWorld);

		ImGui::SetCursorPos(startPos);
		RenderToolbar(levelWorld);
	}

	void LevelViewPanel::ProcessEvent(Event& e)
	{
		ViewPanelBase::ProcessEvent(e);

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>(ZE_BIND_EVENT_FUNC(LevelViewPanel::OnKeyPressed));
		dispatcher.Dispatch<MouseButtonPressedEvent>(ZE_BIND_EVENT_FUNC(LevelViewPanel::OnMouseButtonPressed));
	}

	void LevelViewPanel::OnWorldChanged(const Ref<EditorPreviewWorldBase>& world, const Ref<EditorPreviewWorldBase>& lastWorld)
	{
		ViewPanelBase::OnWorldChanged(world, lastWorld);

		if (lastWorld)
		{
			lastWorld->m_OnActiveSceneChanged.disconnect<&LevelViewPanel::OnActiveSceneChanged>(this);
		}

		world->m_OnActiveSceneChanged.connect<&LevelViewPanel::OnActiveSceneChanged>(this);
	}

	void LevelViewPanel::OnActiveSceneChanged(const Ref<Scene>& scene, const Ref<Scene>& lastScene)
	{
		if (lastScene)
		{
			scene->UnbindOnComponentConstruct<CameraComponent, &ViewPanelBase::UpdateViewportSizeOnSceneCameras>(this);
		}
		BindCameraComponentConstructionDelegate(scene);
	}

	void LevelViewPanel::BindCameraComponentConstructionDelegate(const Ref<Scene>& scene)
	{
		scene->BindOnComponentConstruct<CameraComponent, &ViewPanelBase::UpdateViewportSizeOnSceneCameras>(this);
	}

	void LevelViewPanel::RenderToolbar(const Ref<LevelPreviewWorld>& levelWorld) const
	{
		// Place buttons at window center
		float indent = levelWorld->GetSceneState() > SceneState::Edit ? ImGui::GetContentRegionAvail().x * 0.5f - ImGui::GetFrameHeightWithSpacing() :
			(ImGui::GetContentRegionAvail().x - ImGui::GetFontSize()) * 0.5f - ImGui::GetFramePadding().x;
		ImGui::Indent(indent);

		// Toggle play / stop
		if (ImGui::TransparentButton(levelWorld->GetSceneState() > SceneState::Edit ? ICON_FA_STOP : ICON_FA_PLAY))
		{
			switch (levelWorld->GetSceneState())
			{
				case SceneState::Edit:
					levelWorld->OnScenePlay();
					break;
				case SceneState::Play:
				case SceneState::Pause:
					levelWorld->OnSceneStop();
					break;
			}
		}

		// Toggle pause / resume
		switch (levelWorld->GetSceneState())
		{
			case SceneState::Play:
				ImGui::SameLine();
				if (ImGui::TransparentButton(ICON_FA_PAUSE))
				{
					levelWorld->OnScenePause();
				}
				break;
			case SceneState::Pause:
				ImGui::SameLine();
				if (ImGui::TransparentButton(ICON_FA_PLAY))
				{
					levelWorld->OnSceneResume();
				}
				break;
		}
	}

	void LevelViewPanel::RenderGizmo(const Ref<LevelPreviewWorld>& levelWorld)
	{
		Entity selectedEntity = levelWorld->GetContextEntity();
		m_bGizmoVisible = selectedEntity && m_GizmoType != -1;
		if (m_bGizmoVisible)
		{
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();
			const Vec2* viewportBounds = GetViewportBounds();
			const Vec2 viewportSize = GetViewportSize();
			ImGuizmo::SetRect(viewportBounds[0].x, viewportBounds[0].y, viewportSize.x, viewportSize.y);

			// Editor camera
			const auto& editorCamera = levelWorld->GetEditorCamera();
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
				Vec3 deltaRotation = outRotation - transformComp.Rotation;
				selectedEntity.SetTransform(outTranslation, transformComp.Rotation + deltaRotation, outScale);
			}
		}
	}

	bool LevelViewPanel::OnKeyPressed(KeyPressedEvent& e)
	{
		const auto levelWorld = m_LevelWorld.lock();
		bool bIsCtrlPressed = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		bool bIsAltPressed = Input::IsKeyPressed(Key::LeftAlt) || Input::IsKeyPressed(Key::RightAlt);

		// Global responsing shotcuts
		{
			switch (e.GetKeyCode())
			{
				// Toggle play
				case Key::P:
				{
					if (bIsAltPressed)
					{
						if (levelWorld->GetSceneState() == SceneState::Edit)
						{
							levelWorld->OnScenePlay();
							return true;
						}
					}
					break;
				}
				// Toggle pause / resume
				case Key::Pause:
				{
					switch (levelWorld->GetSceneState())
					{
						case SceneState::Play:	levelWorld->OnScenePause();	return true;
						case SceneState::Pause:	levelWorld->OnSceneResume();	return true;
					}
				}
				// Toggle stop
				case Key::Escape:
				{
					if (levelWorld->GetSceneState() > SceneState::Edit)
					{
						levelWorld->OnSceneStop();
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
					levelWorld->OnDeleteEntity();
					return true;
				}
				case Key::D:
				{
					if (bIsCtrlPressed)
					{
						levelWorld->OnDuplicateEntity();
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

	void LevelViewPanel::ReadPixelDataFromIDBuffer(const Ref<FrameBuffer>& frameBuffer)
	{
		auto [mx, my] = GetMouseViewportPosition();
		Vec2 viewportSize = GetViewportBounds()[1] - GetViewportBounds()[0];
		// Y coordinate for framebuffer texture is from bottom to up
		my = viewportSize.y - my;
		if (mx >= 0.0f && my >= 0.0f && mx < viewportSize.x && my < viewportSize.y)
		{
			Vec4 pixel;
			frameBuffer->ReadPixel(1, static_cast<I32>(mx), static_cast<I32>(my), glm::value_ptr(pixel));
			m_HoveredEntity = pixel.x == -1 ? Entity{} : Entity(static_cast<entt::entity>(pixel.x), GetEditorWorld()->GetActiveScene());

			auto& Stats = Renderer::GetStats();
			Stats.HoveredEntity = m_HoveredEntity;
		}
	}

}

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
#include "Engine/ImGui/EditorConsole.h"
#include "Engine/Physics/PhysXScene.h"
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
		dispatcher.Dispatch<KeyPressedEvent>([this](KeyPressedEvent& e) { return OnKeyPressed(e); });
		dispatcher.Dispatch<MouseButtonPressedEvent>([this](MouseButtonPressedEvent& e) { return OnMouseButtonPressed(e); });
	}

	void LevelViewPanel::RenderToolbar()
	{
		auto* levelWorld = static_cast<LevelPreviewWorld*>(GetEditorWorld());

		// Place buttons at window center
		float indent = levelWorld->GetSceneState() == SceneState::Pause
			? (ImGui::GetContentRegionAvail().x - ImGui::GetFontSize() * 2) * 0.5f - ImGui::GetFramePadding().x * 2 // 3 buttons
			: (ImGui::GetContentRegionAvail().x - ImGui::GetFontSize()) * 0.5f - ImGui::GetFramePadding().x; // 2 buttons

		ImGui::Indent(indent);

		auto Stop = [levelWorld]()
		{
			// Stop simulate / play
			if (ImGui::TransparentButton(ICON_FA_STOP))
			{
				if (levelWorld->IsSimulation())
				{
					levelWorld->OnSceneStopSimulation();
				}
				else
				{
					levelWorld->StopScene();
				}
			}

			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltipWithPadding("Stop the scene (Esc)");
			}
		};

		switch (levelWorld->GetSceneState())
		{
			case SceneState::Edit:
				// Play
				if (ImGui::TransparentButton(ICON_FA_PLAY))
				{
					levelWorld->OnScenePlay();
				}
				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltipWithPadding("Play the scene (Alt-P)");
				}

				ImGui::SameLine();

				// Simulate
				if (ImGui::TransparentButton(ICON_FA_PLAY_CIRCLE))
				{
					levelWorld->OnSceneStartSimulation();
				}

				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltipWithPadding("Simulate the scene (Ctrl-P)");
				}

				break;
			case SceneState::Run:
				Stop();

				ImGui::SameLine();

				// Pause
				if (ImGui::TransparentButton(ICON_FA_PAUSE))
				{
					levelWorld->OnScenePause();
				}

				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltipWithPadding("Pause the scene (Pause)");
				}
				break;
			case SceneState::Pause:
				Stop();

				ImGui::SameLine();

				// Resume
				if (ImGui::TransparentButton(ICON_FA_PLAY))
				{
					levelWorld->OnSceneResume();
				}

				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltipWithPadding("Resume the scene (Pause)");
				}

				ImGui::SameLine();

				// Step one frame
				if (ImGui::TransparentButton(ICON_FA_STEP_FORWARD))
				{
					levelWorld->OnSceneStep();
				}

				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltipWithPadding("Step one frame (F10)");
				}
				break;
		}

		ImGui::SameLine();
		
		// Edit bar
		if (levelWorld->GetSceneState() == SceneState::Edit || levelWorld->IsSimulation())
		{
			// Align to the right
			indent = ImGui::GetContentRegionAvail().x - ImGui::GetFontSize() * 4;
			ImGui::Indent(indent);
			if (ImGui::ActiveTransparentButton(ICON_FA_ARROWS_ALT, m_GizmoType == ImGuizmo::TRANSLATE))
			{
				SetTranslationGizmo();
			}

			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltipWithPadding("Translation (W)");
			}

			ImGui::SameLine();

			if (ImGui::ActiveTransparentButton(ICON_FA_SYNC_ALT, m_GizmoType == ImGuizmo::ROTATE))
			{
				SetRotationGizmo();
			}

			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltipWithPadding("Rotation (E)");
			}

			ImGui::SameLine();

			if (ImGui::ActiveTransparentButton(ICON_FA_EXTERNAL_LINK_SQUARE_ALT, m_GizmoType == ImGuizmo::SCALE))
			{
				SetScaleGizmo();
			}

			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltipWithPadding("Scale (R)");
			}

			ImGui::SameLine();

			if (ImGui::TransparentButton(m_GizmoMode == ImGuizmo::LOCAL ? ICON_FA_DICE_D6 : ICON_FA_GLOBE))
			{
				ToggleGizmoMode();
			}

			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltipWithPadding("World Space / Local Space (T)");
			}
		}
	}

	static void PutPhysicsActorsToSleep(const Scene& scene, const PhysXScene* physicsScene, const Entity& entity)
	{
		if (const auto* actor = physicsScene->GetActor(entity))
		{
			actor->PutToSleep();

			for (const UUID childID : entity.GetChildren())
			{
				if (const Entity child = scene.GetEntityByUUID(childID))
				{
					PutPhysicsActorsToSleep(scene, physicsScene, child);
				}
			}
		}
	}

	void LevelViewPanel::RenderGizmo()
	{
		auto* levelWorld = static_cast<LevelPreviewWorld*>(GetEditorWorld());

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
			Mat4 worldTransform = selectedEntity.GetWorldTransform();

			// Snapping
			bool bSnap = Input::IsKeyPressed(Key::LeftControl);
			// Snap to 0.5m for translation/scale
			float snapValue = 0.5f;
			// Snap to 45 degrees for rotation
			if (m_GizmoType == ImGuizmo::ROTATE)
			{
				snapValue = 45.0f;
			}
			const float snapValues[3] = { snapValue, snapValue, snapValue };

			ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
				m_GizmoType, m_GizmoMode,
				glm::value_ptr(worldTransform),
				nullptr, bSnap ? snapValues : nullptr);

			if (ImGuizmo::IsUsing())
			{
				// When manipulating gizmo at runtime, we temporarily "disable" physics on relevant physics actors so that moving entities will not be affected by physics simulation
				// As transforms are sent to physics system and physics actors are woken up every frame during manipulation, those sleeping entities will eventually wake up when manipulation ends
				// See UpdatePhysicsActorsTransform in LevelObserverSystem::OnUpdate
				//const auto scene = levelWorld->GetActiveScene();
				//if (const auto* physicsScene = scene->GetPhysicsScene())
				//{
				//	PutPhysicsActorsToSleep(*scene, physicsScene, selectedEntity);
				//}

				Mat4 localTransform = worldTransform;
				if (const Entity parent = selectedEntity.GetParentEntity())
				{
					const Mat4 parentWorldTransform = parent.GetWorldTransform();
					localTransform = glm::inverse(parentWorldTransform) * worldTransform;
				}

				Vec3 translation, rotation, scale;
				Math::DecomposeTransform(localTransform, translation, rotation, scale);

				// This delta rotation prevents gimbal lock situation
				const auto& transformComp = selectedEntity.GetComponent<TransformComponent>();
				const Vec3 deltaRotation = rotation - transformComp.GetRotationInRadians();
				selectedEntity.SetTransform(translation, transformComp.GetRotationInRadians() + deltaRotation, scale);
			}
		}
	}

	bool LevelViewPanel::OnKeyPressed(KeyPressedEvent& e)
	{
		auto* levelWorld = static_cast<LevelPreviewWorld*>(GetEditorWorld());

		bool bIsCtrlPressed = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		bool bIsAltPressed = Input::IsKeyPressed(Key::LeftAlt) || Input::IsKeyPressed(Key::RightAlt);

		// TODO: Global shortcuts
		{
			switch (e.GetKeyCode())
			{
				// Toggle play
				case Key::P:
				{
					if (!levelWorld->IsRuntime())
					{
						if (bIsAltPressed)
						{
							levelWorld->OnScenePlay();
							return true;
						}
						if (bIsCtrlPressed)
						{
							levelWorld->OnSceneStartSimulation();
							return true;
						}
					}
					break;
				}
				// Step a frame
				case Key::F10:
				{
					if (levelWorld->GetSceneState() == SceneState::Pause)
					{
						levelWorld->OnSceneStep();
						return true;
					}
					break;
				}
				// Toggle pause / resume
				case Key::Pause:
				{
					switch (levelWorld->GetSceneState())
					{
						case SceneState::Run:	levelWorld->OnScenePause();	return true;
						case SceneState::Pause:	levelWorld->OnSceneResume();	return true;
					}
				}
				// Toggle stop
				case Key::Escape:
				{
					if (levelWorld->IsRuntime())
					{
						if (levelWorld->IsSimulation())
						{
							levelWorld->OnSceneStopSimulation();
						}
						else
						{
							levelWorld->OnSceneStop();
						}
						return true;
					}
					break;
				}
				case Key::F9:
				{
					if (auto* renderDoc = Application::Get().GetRenderDoc())
					{
						renderDoc->ToggleEnableCapture();
					}
					break;
				}
				// Toggle console command input
				case Key::GraveAccent:
				{
					if (ImGui::GetActiveID() == EditorConsole::s_Instance.ConsoleInputItemID)
					{
						ImGui::ClearActiveID();
					}
					else
					{
						EditorConsole::s_Instance.bRequestKeyboardFocus = true;
					}
					break;
				}
			}
		}

		if ((!IsPanelFocused() && !IsPanelHovered()) || e.GetRepeatCount() > 0) return false;
		if (levelWorld->IsRuntime() && !levelWorld->IsSimulation()) return false;

		const auto& editorCamera = levelWorld->GetEditorCamera();
		if (!editorCamera.IsManipulating())
		{
			switch (e.GetKeyCode())
			{
				case Key::W:
				{
					SetTranslationGizmo();
					return true;
				}
				case Key::E:
				{
					SetRotationGizmo();
					return true;
				}
				case Key::R:
				{
					SetScaleGizmo();
					return true;
				}
				case Key::Space:
				{
					ToggleGizmoType();
					return true;
				}
				case Key::T:
				{
					ToggleGizmoMode();
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
		if (!IsPanelHovered()) return false;

		// We definitely do not want to deselect entity when clicking on transform buttons
		if (ImGui::IsAnyItemHovered()) return false;

		if (e.GetMouseButton() != Mouse::ButtonLeft) return false;

		if (Input::IsKeyPressed(Key::CameraControl)) return false;

		if (m_bGizmoVisible && ImGuizmo::IsOver()) return false;

		GetEditorWorld()->SetContextEntity(m_HoveredEntity);
		return true;
	}

	void LevelViewPanel::SetTranslationGizmo()
	{
		if (!ImGuizmo::IsUsing())
		{
			m_GizmoType = ImGuizmo::TRANSLATE;
		}
	}

	void LevelViewPanel::SetRotationGizmo()
	{
		if (!ImGuizmo::IsUsing())
		{
			m_GizmoType = ImGuizmo::ROTATE;
		}
	}

	void LevelViewPanel::SetScaleGizmo()
	{
		if (!ImGuizmo::IsUsing())
		{
			m_GizmoType = ImGuizmo::SCALE;
		}
	}

	void LevelViewPanel::ToggleGizmoType()
	{
		if (!ImGuizmo::IsUsing())
		{
			switch (m_GizmoType)
			{
				case ImGuizmo::TRANSLATE:	m_GizmoType = ImGuizmo::ROTATE;		break;
				case ImGuizmo::ROTATE:		m_GizmoType = ImGuizmo::SCALE;		break;
				case ImGuizmo::SCALE:		m_GizmoType = ImGuizmo::TRANSLATE;	break;
			}
		}
	}

	void LevelViewPanel::ToggleGizmoMode()
	{
		if (!ImGuizmo::IsUsing())
		{
			m_GizmoMode = m_GizmoMode == ImGuizmo::LOCAL ? ImGuizmo::WORLD : ImGuizmo::LOCAL;
		}
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

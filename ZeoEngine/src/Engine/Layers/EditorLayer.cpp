#include "ZEpch.h"
#include "Engine/Layers/EditorLayer.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "Engine/Renderer/Renderer2D.h"
#include "Engine/Renderer/RenderCommand.h"
#include "Engine/Core/Application.h"
#include "Engine/Layers/GameLayer.h"
#include "Engine/Core/EngineGlobals.h"
#include "Engine/GameFramework/Level.h"

namespace ZeoEngine {

	void EditorLayer::OnAttach()
	{

	}

	void EditorLayer::OnUpdate(DeltaTime dt)
	{
		
	}

	void EditorLayer::OnImGuiRender()
	{
		//bool bShow = false;
		//ImGui::ShowDemoWindow(&bShow);

		// TODO: These bools are not saved out, so last opened windows cannot restore
		static bool bShowGameView = true;
		static bool bShowLevelOutline = true;
		static bool bShowConsole = true;
		static bool bShowPreferences = false;

		ShowEditorDockspace();

		if (ImGui::BeginMainMenuBar())
		{
			// File menu
			if (ImGui::BeginMenu("File"))
			{
				ImGui::EndMenu();
			}
			// Edit menu
			if (ImGui::BeginMenu("Edit"))
			{
				if (ImGui::MenuItem("Undo", "CTRL+Z"))
				{

				}
				if (ImGui::MenuItem("Redo", "CTRL+Y", false, false))
				{

				}
				ImGui::Separator();
				if (ImGui::MenuItem("Copy", "CTRL+C"))
				{

				}
				if (ImGui::MenuItem("Paste", "CTRL+V"))
				{

				}
				if (ImGui::MenuItem("Cut", "CTRL+X"))
				{

				}
				ImGui::Separator();
				ImGui::MenuItem("Preferences", nullptr, &bShowPreferences);
				ImGui::EndMenu();
			}
			// Window menu
			if (ImGui::BeginMenu("Window"))
			{
				ImGui::MenuItem("Game View", nullptr, &bShowGameView);
				ImGui::MenuItem("Level Outline", nullptr, &bShowLevelOutline);
				ImGui::MenuItem("Console", nullptr, &bShowConsole);
				ImGui::Separator();
				// Reset layout on next frame
				if (ImGui::MenuItem("Reset layout", nullptr))
				{
					m_bResetLayout = true;
				}
				ImGui::EndMenu();
			}
			// Display engine stats at right corner of menu bar
			const float statsWidth = 125.0f;
			ImGui::Indent(ImGui::GetWindowSize().x - statsWidth);
			ImGui::Text("%.f FPS (%.2f ms)", ImGui::GetIO().Framerate, 1000.f / ImGui::GetIO().Framerate);
			ImGui::EndMainMenuBar();
		}

		if (bShowGameView)
		{
			ShowGameView(&bShowGameView);
		}
		else
		{
			// TODO: disable framebuffer rendering

		}
		if (bShowLevelOutline)
		{
			ShowLevelOutline(&bShowLevelOutline);
		}
		if (bShowConsole)
		{
			ShowConsole(&bShowConsole);
		}

		if (bShowPreferences)
		{
			ShowPreferences(&bShowPreferences);
		}
	}

	void EditorLayer::ShowEditorDockspace()
	{
		ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		static bool bOpen = true;
		ImGui::Begin("Editor", &bOpen, windowFlags);
		ImGui::PopStyleVar(3);
		ImGuiID editorDockspaceId = ImGui::GetID("EditorDockspace");
		// Update docking layout
		if (ImGui::DockBuilderGetNode(editorDockspaceId) == nullptr || m_bResetLayout)
		{
			m_bResetLayout = false;

			// Clear out existing layout
			ImGui::DockBuilderRemoveNode(editorDockspaceId);
			// Add empty node
			ImGui::DockBuilderAddNode(editorDockspaceId, ImGuiDockNodeFlags_DockSpace);
			const auto& window = Application::Get().GetWindow();
			// Main node should cover entire screen
			ImGui::DockBuilderSetNodeSize(editorDockspaceId, ImVec2((float)window.GetWidth(), (float)window.GetHeight()));

			ImGuiID dockMainUp;
			ImGuiID dockMainDown = ImGui::DockBuilderSplitNode(editorDockspaceId, ImGuiDir_Down, 0.3f, nullptr, &dockMainUp);
			ImGuiID dockMainRight;
			ImGuiID dockMainLeft = ImGui::DockBuilderSplitNode(dockMainUp, ImGuiDir_Left, 0.8f, nullptr, &dockMainRight);

			ImGui::DockBuilderDockWindow("Game View", dockMainLeft);
			ImGui::DockBuilderDockWindow("Level Outline", dockMainRight);
			ImGui::DockBuilderDockWindow("Console", dockMainDown);

			ImGui::DockBuilderFinish(editorDockspaceId);
		}
		// Should be put at last
		ImGui::DockSpace(editorDockspaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
		ImGui::End();
	}

	void EditorLayer::ShowGameView(bool* bShow)
	{
		if (ImGui::Begin("Game View", bShow))
		{
			auto window = ImGui::GetCurrentWindow();
			// TODO: Add conversion from ImVec2 to glm::vec2 inside imconfig.h
			glm::vec2 max = { window->InnerRect.Max.x, window->InnerRect.Max.y };
			glm::vec2 min = { window->InnerRect.Min.x, window->InnerRect.Min.y };
			glm::vec2 size = max - min;
			// Update camera aspect ratio when game view window is resized
			if (size != m_LastGameViewSize)
			{
				GameLayer* gl = Application::Get().FindLayerByName<GameLayer>("Game");
				if (gl)
				{
					gl->CreateCamera((uint32_t)size.x, (uint32_t)size.y);
				}
			}
			ImGui::GetWindowDrawList()->AddImage(
				Renderer2D::GetStorageData()->MainFBO->GetRenderedTexture(),
				// Upper left corner for the UVs to be applied at
				window->InnerRect.Min,
				// Lower right corner for the UVs to be applied at
				window->InnerRect.Max,
				// The UVs have to be flipped
				ImVec2(0, 1), ImVec2(1, 0));

			max = { window->InnerRect.Max.x, window->InnerRect.Max.y };
			min = { window->InnerRect.Min.x, window->InnerRect.Min.y };
			m_LastGameViewSize = max - min;
		}
		ImGui::End();
	}

	void EditorLayer::ShowLevelOutline(bool* bShow)
	{
		Level* level = GetLevel<Level>();
		if (!level)
			return;

		if (ImGui::Begin("Level Outline", bShow))
		{
			ImGui::Text("(%d objects total)", level->m_GameObjects.size());
			for (uint32_t i = 0; i < level->m_GameObjects.size(); ++i)
			{
				ImVec4 color;
				if (level->m_GameObjects[i]->IsActive())
				{
					// Translucent objects are marked yellow instead of white
					color = level->m_GameObjects[i]->IsTranslucent() ? ImVec4(1.0f, 1.0f, 0.0f, 1.0f) : ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
				}
				else
				{
					// Inactive objects are marked darker
					color = level->m_GameObjects[i]->IsTranslucent() ? ImVec4(0.75f, 0.75f, 0.0f, 1.0f) : ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
				}
				ImGui::TextColored(color, "%s", level->m_GameObjects[i]->GetName().c_str());
			}
		}
		ImGui::End();
	}

	void EditorLayer::ShowConsole(bool* bShow)
	{
		ImGui::Begin("Console", bShow);
		ImGui::End();
	}

	void EditorLayer::ShowPreferences(bool* bShow)
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x + 100, viewport->Pos.y + 100), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);
		if (ImGui::Begin("Preferences", bShow, ImGuiWindowFlags_NoCollapse))
		{
			ImGui::ShowStyleSelector("Editor style");
		}
		ImGui::End();
	}

}


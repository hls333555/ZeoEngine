#pragma once

#include "Engine/ImGui/MyImGui.h"

namespace ZeoEngine {

	enum class EditorDockspaceType
	{
		NONE,

		Main_Editor,
		Particle_Editor,
	};

	enum class EditorPanelType
	{
		NONE,

		// Main editor windows
		Game_View,
		Scene_Outline,
		Entity_Inspector,
		Console,
		Stats,
		Preferences,
		About,

		// Particle editor windows
		Particle_View,
		Particle_Inspector,
	};

	struct EditorDockspaceSpec
	{
		EditorDockspaceType Type;
		glm::vec2 Padding{ 0.0f };
		ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_MenuBar;
		ImVec2Data InitialSize{ { 800.0f, 600.0f } };
	};

	struct EditorPanelSpec
	{
		EditorPanelType Type;
		ImGuiWindowFlags WindowFlags = 0;
		ImVec2Data InitialSize{ { 800.0f, 600.0f } };
	};

}

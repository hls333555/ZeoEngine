#pragma once

#include "Engine/ImGui/MyImGui.h"

namespace ZeoEngine {

	enum class EditorType
	{
		NONE,

		SceneEditor,
		ParticleEditor,
	};

	enum class PanelType
	{
		NONE,

		// Main editor windows
		GameView,
		SceneOutline,
		EntityInspector,
		ContentBrowser,
		Console,
		Stats,
		Preferences,
		About,

		// Particle editor windows
		ParticleView,
		ParticleInspector,
	};

	struct DockspaceSpec
	{
		ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_MenuBar;
		ImVec2Data InitialSize{ { 800.0f, 600.0f } };
	};

	struct PanelSpec
	{
		ImGuiWindowFlags WindowFlags = 0;
		ImVec2 Padding = ImVec2(3.0f, 3.0f);
		ImVec2Data InitialSize{ { 800.0f, 600.0f } };
	};

}

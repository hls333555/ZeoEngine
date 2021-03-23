#pragma once

#include "Engine/ImGui/MyImGui.h"

namespace ZeoEngine {

	enum class DockspaceType
	{
		NONE,

		MainEditor,
		ParticleEditor,
	};

	enum class PanelType
	{
		NONE,

		// Main editor windows
		GameView,
		SceneOutline,
		EntityInspector,
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
		DockspaceType Type;
		ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_MenuBar;
		ImVec2Data InitialSize{ { 800.0f, 600.0f } };
	};

	struct PanelSpec
	{
		PanelType Type;
		ImGuiWindowFlags WindowFlags = 0;
		ImVec2 Padding = ImVec2(3.0f, 3.0f);
		ImVec2Data InitialSize{ { 800.0f, 600.0f } };
	};

}

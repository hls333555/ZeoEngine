#pragma once

#include <string>

#include <magic_enum.hpp>

namespace ZeoEngine {

	// NOTE: All enums should use '_' to concatenate words
	enum class EditorDockspaceType
	{
		NONE,

		Main_Editor,
		Particle_Editor,
	};

	// NOTE: All enums should use '_' to concatenate words
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

	template<typename T>
	std::string ResolveEditorNameFromEnum(T type)
	{
		std::string name = magic_enum::enum_name(type).data();
		for (std::string::size_type pos{ 0 }; pos != std::string::npos; ++pos) {
			if ((pos = name.find("_", pos)) == std::string::npos) break;

			name.replace(pos, 1, " ");
		}
		return name;
	}

}

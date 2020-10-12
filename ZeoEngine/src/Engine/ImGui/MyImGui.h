#pragma once

#include <imgui.h>
#include <glm/glm.hpp>

namespace ImGui {

	IMGUI_API void TextCentered(const char* fmt, ...);

	/** To draw a progress bar in Game View window, use GetWindowDrawList(); to draw a progress bar in standalone game, use GetForgroundDrawList(). */
	IMGUI_API void AddProgressBar(ImDrawList* drawList, float fraction, const ImVec2& a, const ImVec2& b, ImU32 foregroundCol, ImU32 backgroundCol);
}

namespace ZeoEngine {
	
	struct ImVec2Data
	{
		glm::vec2 Data;
		ImGuiCond Condition{ ImGuiCond_FirstUseEver };

		bool operator==(const ImVec2Data& other) const
		{
			return Data.x == other.Data.x && Data.y == other.Data.y && Condition == other.Condition;
		}

		static ImVec2Data DefaultPos; // Center of main viewport
		static ImVec2Data DefaultSize;
	};

}

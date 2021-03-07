#pragma once

#include <imgui.h>

namespace ImGui {

	IMGUI_API void TextCentered(const char* fmt, ...);

	/** Version that displays colored lines under each component (components > 1). */
	IMGUI_API bool DragScalarNEx(const char* label, ImGuiDataType data_type, void* p_data, int components, float v_speed, const void* p_min, const void* p_max, const char* format, ImGuiSliderFlags flags);
}

namespace ZeoEngine {
	
	struct ImVec2Data
	{
		ImVec2 Data;
		ImGuiCond Condition{ ImGuiCond_FirstUseEver };

		bool operator==(const ImVec2Data& other) const
		{
			return Data.x == other.Data.x && Data.y == other.Data.y && Condition == other.Condition;
		}
	};

}

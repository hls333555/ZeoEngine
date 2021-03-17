#pragma once

#include <imgui.h>

namespace ImGui {

	IMGUI_API void TextCentered(const char* fmt, ...);

	/** Version that displays tooltip well with rounding corner. */
	IMGUI_API void SetTooltipRounded(const char* fmt, ...) IM_FMTARGS(1);

	IMGUI_API void PushMultiItemsWidthsWithLabels(int components, float label_width, float w_full);
	/** Version that displays (x,y,z) labels before each component (components > 1). */
	IMGUI_API bool DragScalarNEx(const char* label, ImGuiDataType data_type, void* p_data, int components, float v_speed, const void* p_min, const void* p_max, const char* format, ImGuiSliderFlags flags);

	/** Version that only renders text by default. */
	IMGUI_API bool TransparentSmallButton(const char* label);
	IMGUI_API bool TransparentButton(const char* label, const ImVec2& size = ImVec2(0, 0));
	IMGUI_API bool TransparentButtonEx(const char* label, const ImVec2& size_arg = ImVec2(0, 0), ImGuiButtonFlags flags = 0);

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

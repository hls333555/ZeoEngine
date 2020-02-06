#pragma once

#include <imgui.h>

namespace ImGui {

	IMGUI_API bool DragInt_8(const char* label, int8_t* v, float v_speed = 1.0f, int8_t v_min = 0, int8_t v_max = 0, const char* format = "%d");
	IMGUI_API bool DragInt_32(const char* label, int32_t* v, float v_speed = 1.0f, int32_t v_min = 0, int32_t v_max = 0, const char* format = "%d");
	IMGUI_API bool DragInt_64(const char* label, int64_t* v, float v_speed = 1.0f, int64_t v_min = 0, int64_t v_max = 0, const char* format = "%d");
	IMGUI_API bool DragUInt_8(const char* label, uint8_t* v, float v_speed = 1.0f, uint8_t v_min = 0, uint8_t v_max = 0, const char* format = "%d");
	IMGUI_API bool DragUInt_32(const char* label, uint32_t* v, float v_speed = 1.0f, uint32_t v_min = 0, uint32_t v_max = 0, const char* format = "%d");
	IMGUI_API bool DragUInt_64(const char* label, uint64_t* v, float v_speed = 1.0f, uint64_t v_min = 0, uint64_t v_max = 0, const char* format = "%d");
	IMGUI_API bool DragDouble(const char* label, double* v, float v_speed = 1.0f, double v_min = 0.0f, double v_max = 0.0f, const char* format = "%.3f", float power = 1.0f);

}
